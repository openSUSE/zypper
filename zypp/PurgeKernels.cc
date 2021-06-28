/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PurgeKernels.cc
 *
*/

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Regex.h>
#include <zypp/base/Iterator.h>
#include <zypp/PurgeKernels.h>
#include <zypp/PoolQuery.h>
#include <zypp/ResPool.h>
#include <zypp/Resolver.h>
#include <zypp/Filter.h>
#include <zypp/ZConfig.h>

#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <sys/utsname.h>
#include <functional>
#include <array>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "PurgeKernels"

namespace zypp {

  using Flavour                = std::string;
  using SolvableList           = std::list<sat::Solvable>;
  using EditionToSolvableMap   = std::map<Edition, SolvableList >;
  using ArchToEditionMap       = std::map<Arch, EditionToSolvableMap >;

  struct GroupInfo {

    enum GroupType {
      None,             //<< Just here to support default construction
      Kernels,          //<< Map contains kernel packages, so need to receive special handling and flavour matching
      RelatedBinaries,  //<< Map contains related binary packages, so need to receive special handling and flavour matching
      Sources           //<< Map contains source packages, so when matching those against running we ignore the flavour
    } groupType;

    GroupInfo( const GroupType type = None, std::string flav = "") : groupType(type), groupFlavour( std::move(flav) ) { }

    ArchToEditionMap archToEdMap;   //<< Map of actual packages
    std::string groupFlavour;       //<< This would contain a specific flavour if there is one calculated
  };
  using GroupMap = std::unordered_map<std::string, GroupInfo>;

  struct PurgeKernels::Impl  {

    Impl() {
      struct utsname unameData;
      if ( uname( &unameData) == 0 ) {

        const auto archStr = str::regex_substitute( unameData.machine, str::regex( "^i.86$", str::regex::match_extended ), "i586" );

        _kernelArch = Arch( archStr );
        setUnameR( std::string( unameData.release ) );

        _detectedRunning = true;

        MIL << "Detected running kernel: Flavour: " << _runningKernelFlavour << " Arch: " << _kernelArch << "\n";
        for ( const auto &edVar : _runningKernelEditionVariants )
          MIL << "Edition variant: " << edVar << "\n";
        MIL << std::endl;

      } else {
        MIL << "Failed to detect running kernel: " << errno << std::endl;
      }
    }

    void setUnameR ( const std::string &uname ) {

      _uname_r = uname;

      MIL << "Set uname " << uname << std::endl;

      const std::string flavour = str::regex_substitute( _uname_r, str::regex( ".*-", str::regex::match_extended ), "", true );
      std::string version = str::regex_substitute( _uname_r, str::regex( "-[^-]*$", str::regex::match_extended | str::regex::newline ), "", true );

      const std::string release = str::regex_substitute( version, str::regex( ".*-", str::regex::match_extended ), "", true );

      version = str::regex_substitute( version, str::regex( "-[^-]*$", str::regex::match_extended | str::regex::newline ), "", true );


      auto makeRcVariant = [ &release ]( std::string myVersion, const std::string &replace ){
        // from purge-kernels script, was copied from kernel-source/rpm/mkspec
        myVersion = str::regex_substitute( myVersion, str::regex( "\\.0-rc", str::regex::match_extended ), replace, true );
        myVersion = str::regex_substitute( myVersion, str::regex( "-rc\\d+", str::regex::match_extended ), "", true );
        myVersion = str::regex_substitute( myVersion, str::regex( "-", str::regex::match_extended ), ".", true );
        return Edition( myVersion, release );
      };

      _runningKernelEditionVariants.clear();
      _runningKernelEditionVariants.insert( makeRcVariant( version, "~rc") );
      _runningKernelEditionVariants.insert( makeRcVariant( version, ".rc") );

      _runningKernelFlavour = flavour;

      MIL << "Parsed info from uname: " << std::endl;
      MIL << "Kernel Flavour: " << _runningKernelFlavour << std::endl;
      MIL << "Kernel Edition Variants: \n";
      for ( const auto &var : _runningKernelEditionVariants )
        MIL << "    " << var << "\n";
      MIL << std::endl;
    }

    bool removePackageAndCheck( const sat::Solvable slv, const std::set<sat::Solvable> &keepList , const std::set<sat::Solvable> &removeList ) const;
    static bool versionMatch ( const Edition &a, const Edition &b );
    void parseKeepSpec();
    void fillKeepList(const GroupMap &installedKernels, std::set<sat::Solvable> &keepList , std::set<sat::Solvable> &removeList ) const;

    std::set<size_t>  _keepLatestOffsets = { 0 };
    std::set<size_t>  _keepOldestOffsets;
    std::set<Edition> _keepSpecificEditions;
    std::string       _uname_r;
    /*!
     *  A kernel uname can expand into multiple variants especially with rc kernels,
     *  older rc releases replaced the upstream tag -rc with .rc, however newer code
     *  replaces it with ~rc. We need to check against all variants to match the correct one.
     */
    std::set<Edition> _runningKernelEditionVariants;
    Flavour           _runningKernelFlavour;
    Arch              _kernelArch;
    std::string       _keepSpec = ZConfig::instance().multiversionKernels();
    bool              _keepRunning     = true;
    bool              _detectedRunning = false;
  };

  /*!
   * tries to remove a the \ref PoolItem \a pi from the pool, solves and checks if no unexpected packages are removed due to the \a validRemovals regex.
   * If the constraint fails the changes are reverted and \a false is returned.
   */
  bool PurgeKernels::Impl::removePackageAndCheck( const sat::Solvable slv, const std::set<sat::Solvable> &keepList , const std::set<sat::Solvable> &removeList ) const
  {
    const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );

    PoolItem pi ( slv );

    auto pool = ResPool::instance();

    // make sure the pool is clean
    if ( !pool.resolver().resolvePool() ) {
      MIL << "Pool failed to resolve, not doing anything" << std::endl;
      return false;
    }

    MIL << "Request to remove package: " << pi << std::endl;

    //list of packages that are allowed to be removed automatically.
    const str::regex validRemovals("(kernel-syms(-.*)?|kgraft-patch(-.*)?|kernel-livepatch(-.*)?|.*-kmp(-.*)?)");

    if ( pi.status().isLocked() ) {
      MIL << "Package " << pi << " is locked by the user, not removing." << std::endl;
      return false;
    }

    //remember which packages are already marked for removal, we do not need to check them again
    std::set<sat::Solvable> currentSetOfRemovals;
    for ( const PoolItem & p : pool.byStatus( toBeUninstalledFilter ) ) {
      currentSetOfRemovals.insert( p.satSolvable() );
    }

    pi.status().setToBeUninstalled( ResStatus::USER );

    if ( !pool.resolver().resolvePool() ) {
      MIL << "Failed to resolve pool, skipping " << pi << std::endl;
      pool.resolver().problems();
      pi.statusReset();

      return false;
    }

    std::set<sat::Solvable> removedInThisRun;
    removedInThisRun.insert( slv );

    for ( const PoolItem & p : pool.byStatus( toBeUninstalledFilter ) ) {

      //check if that package is removeable
      if ( p.status().isByUser()      //this was set by us, ignore it
           || (currentSetOfRemovals.find( p.satSolvable() ) != currentSetOfRemovals.end()) //this was marked by a previous removal, ignore them
        )
        continue;

      // remember for later we need remove the debugsource and debuginfo packages as well
      removedInThisRun.insert( p.satSolvable() );

      MIL << "Package " << p << " was marked by the solver for removal." << std::endl;

      // if we do not plan to remove that package anyway, we need to check if its allowed to be removed ( package in removelist can never be in keep list )
      if ( removeList.find( p.satSolvable() ) != removeList.end() )
        continue;

      if ( keepList.find( p.satSolvable() ) != keepList.end() ) {
        MIL << "Package " << p << " is in keep spec, skipping" << pi << std::endl;
        pi.statusReset();
        return false;
      }

      /*
       * bsc#1185325 We can not solely rely on name matching to figure out
       * which packages are kmod's, in SLES from Leap 15.3 forward we have the
       * kernel-flavour-extra packages ( and others similarly named ) that are basically
       * a collection of kmod's. So checking the name for .*-kmp(-.*)? is not enough.
       * We first check if the package provides kmod(*) or ksym(*) and only fall back to name
       * checking if that is not the case.
       * Just to be safe I'll leave the regex in the fallback case as well, but it should be completely
       * redundant now.
       */
      bool mostLikelyKmod = false;
      StrMatcher matchMod( "kmod(*)", Match::GLOB );
      StrMatcher matchSym( "ksym(*)", Match::GLOB );
      for ( const auto &prov : p.provides() ) {
        if ( matchMod.doMatch( prov.detail().name().c_str()) || matchSym.doMatch( prov.detail().name().c_str() ) ) {
          mostLikelyKmod = true;
          break;
        }
      }

      if ( mostLikelyKmod  ) {
        MIL << "Package " << p << " is most likely a kmod " << std::endl;
      } else {
        str::smatch what;
        if ( !str::regex_match( p.name(), what, validRemovals) ) {
          MIL << "Package " << p << " should not be removed, skipping " << pi << std::endl;
          pi.statusReset();
          return false;
        }
      }
    }

    MIL << "Successfully marked package: " << pi << " for removal."<<std::endl;

    //now check and mark the -debugsource and -debuginfo packages for this package and all the packages that were removed. Maybe collect it before and just remove here
    MIL << "Trying to remove debuginfo for: " << pi <<"."<<std::endl;
    for ( sat::Solvable solvable : removedInThisRun ) {

      if ( solvable.arch() == Arch_noarch ||
           solvable.arch() == Arch_empty )
        continue;

      for ( const char * suffix : { "-debugsource", "-debuginfo" } ) {
        PoolQuery q;
        q.addKind( zypp::ResKind::package );
        q.addDependency( sat::SolvAttr::provides, Capability( solvable.name()+suffix, Rel::EQ, solvable.edition() ) );
        q.setInstalledOnly();
        q.setMatchExact();

        for ( sat::Solvable debugPackage : q ) {

          if ( debugPackage.arch() != solvable.arch() )
            continue;

          MIL << "Found debug package for " << solvable << " : " << debugPackage << std::endl;
          //if removing the package fails it will not stop us from going on , so no need to check
          removePackageAndCheck( debugPackage, keepList, removeList );
        }
      }
    }
    MIL << "Finished removing debuginfo for: " << pi <<"."<<std::endl;

    return true;
  }

  /*!
   * Return true if \a a == \a b or \a a == (\a b minus rebuild counter)
   */
  bool PurgeKernels::Impl::versionMatch( const Edition &a, const Edition &b )
  {
    if ( a == b )
      return true;

    // the build counter should not be considered here, so if there is one we cut it off
    const str::regex buildCntRegex( "\\.[0-9]+($|\\.g[0-9a-f]{7}$)", str::regex::match_extended );

    std::string versionStr = b.asString();
    str::smatch matches;
    if ( buildCntRegex.matches( versionStr.data(), matches ) ) {
      if ( matches.size() >= 2 ) {
        versionStr.replace( matches.begin(0), (matches.end(0) - matches.begin(0))+1, matches[1] );
        return a == Edition(versionStr);
      }
    }
    return false;
  }

  /*!
   * Parse the config line keep spec that tells us which kernels should be kept
   */
  void PurgeKernels::Impl::parseKeepSpec( )
  {
    //keep spec parse regex, make sure to edit the group offsets if changing this regex
    const str::regex specRegex( "^(latest|oldest)([+-][0-9]+)?$", str::regex::match_extended );

    const unsigned tokenGrp = 1; //index of the group matching the token
    const unsigned modifierGrp = 2; //index of the group matching the offset modifier


    MIL << "Parsing keep spec: " << _keepSpec << std::endl;

    std::vector<std::string> words;
    str::split( _keepSpec, std::back_inserter(words), ",", str::TRIM );
    if ( words.empty() ) {
      WAR << "Invalid keep spec: " << _keepSpec << " using default latest,running." << std::endl;
      return;
    }

    _keepRunning = false;
    _keepLatestOffsets.clear();
    _keepOldestOffsets.clear();

    for ( const std::string &word : words ) {
      if ( word == "running" ) {
        _keepRunning = true;
      } else {
        str::smatch what;
        if ( !str::regex_match( word, what, specRegex ) ) {
          _keepSpecificEditions.insert( Edition(word) );
          continue;
        }

        auto addKeepOff = []( const auto &off, auto &set, const auto &constraint ){
          const off_t num = off.empty() ? 0 : str::strtonum<off_t>( off );
          if ( !constraint(num) ) return false;
          set.insert( static_cast<size_t>(std::abs(num)) );
          return true;
        };

        if ( what[tokenGrp] == "oldest" ) {
          addKeepOff( what[modifierGrp], _keepOldestOffsets, [ &word ]( off_t num ) {
            if ( num < 0 ) {
              WAR << "Ignoring invalid modifier in keep spec: " << word << ", oldest supports only positive modifiers." << std::endl;
              return false;
            }
            return true;
          });
        } else {
          addKeepOff( what[modifierGrp], _keepLatestOffsets, [ &word ]( off_t num ) {
            if ( num > 0 ) {
              WAR << "Ignoring invalid modifier in keep spec: " << word << ", latest supports only negative modifiers." << std::endl;
              return false;
            }
            return true;
          });
        }
      }
    }
  }

  /*!
   * Go over the list of available Editions for each flavour/arch combinations, apply the keep spec and mark the
   * packages that belong to a matching category as to keep
   *
   * All packages with Arch_noarch will only be matched against the version but NOT the flavour, reasoning for that is
   * simply that source flavours not necessarily match the binary flavours. Without a translation table that would not be
   * doable. This is also what the perl script did.
   *
   */
  void PurgeKernels::Impl::fillKeepList( const GroupMap &installedKernels, std::set<sat::Solvable> &keepList, std::set<sat::Solvable> &removeList ) const
  {

    const auto markAsKeep = [ &keepList, &removeList ]( sat::Solvable pck ) {
      MIL << "Marking package " << pck << " as to keep." << std::endl;
      keepList.insert( pck ) ;
      removeList.erase( pck );
    };

    const auto versionPredicate = []( const auto &editionVariants ){
      return [ &editionVariants ]( const auto &elem ) {
        const auto &f = std::bind( versionMatch, std::placeholders::_1, elem.first );
        return std::any_of( editionVariants.begin(), editionVariants.end(), f );
      };
    };

    for ( const auto &groupInfo : installedKernels ) {

      MIL << "Starting with group " << groupInfo.first << std::endl;

      for ( const auto &archMap : groupInfo.second.archToEdMap ) {

        MIL << "Starting with arch " << archMap.first << std::endl;

        size_t currOff = 0; //the current "oldest" offset ( runs from map start to end )
        size_t currROff = archMap.second.size() - 1; // the current "latest" offset ( runs from map end to start )


        const EditionToSolvableMap &map = archMap.second;

        if ( _keepRunning
             && ( ( archMap.first == _kernelArch && groupInfo.second.groupFlavour == _runningKernelFlavour )
                  || groupInfo.second.groupType == GroupInfo::Sources ) ) {

          MIL << "Matching packages against running kernel "<< _runningKernelFlavour << "-" <<_kernelArch << "\nVariants:\n";
          for ( const auto &var : _runningKernelEditionVariants )
            MIL << var << "\n";
          MIL << std::endl;

          const auto &editionPredicate = versionPredicate( _runningKernelEditionVariants );
          auto it = std::find_if( map.begin(), map.end(), editionPredicate );
          if ( it == map.end() ) {

            // If we look at Sources we cannot match the flavour but we still want to keep on checking the rest of the keep spec
            if ( groupInfo.second.groupType != GroupInfo::Sources  ) {
              MIL << "Running kernel " << _runningKernelFlavour << "-" <<_kernelArch << "\n";
              for ( const auto &var : _runningKernelEditionVariants )
                MIL << " Possible Variant:" << var << "\n";
              MIL << "Not installed! \n";
              MIL << "NOT removing any packages for flavor "<<_runningKernelFlavour<<"-"<<_kernelArch<<" ."<<std::endl;

              for ( const auto &kernelMap : map ) {
                for( sat::Solvable pck : kernelMap.second )
                  markAsKeep(pck);
              }
              continue;
            }

          } else {
            // there could be multiple matches here because of rebuild counter, lets try to find the last one
            MIL << "Found possible running candidate edition: " << it->first << std::endl;
            auto nit = it;
            for ( nit++ ; nit != map.end() && editionPredicate( *nit ) ; nit++ ) {
              MIL << "Found possible more recent running candidate edition: " << nit->first << std::endl;
              it = nit;
            }
          }

          // mark all packages of the running version as keep
          if ( it != map.end() ) {
            for( sat::Solvable pck : it->second ) {
              markAsKeep(pck);
            }
          }
        }

        for ( const auto &kernelMap : map ) {
          //if we find one of the running offsets in the keepspec, we add the kernel id the the list of packages to keep
          if (  _keepOldestOffsets.find( currOff ) != _keepOldestOffsets.end() || _keepLatestOffsets.find( currROff ) != _keepLatestOffsets.end() ) {
            std::for_each( kernelMap.second.begin(), kernelMap.second.end(), markAsKeep );
          }
          currOff++;
          currROff--;

          // a kernel package might be explicitely locked by version
          // We need to go over all package name provides ( provides is named like the package ) and match
          // them against the specified version to know which ones to keep. (bsc#1176740  bsc#1176192)
          std::for_each( kernelMap.second.begin(), kernelMap.second.end(), [ & ]( sat::Solvable solv ){
            for ( Capability prov : solv.provides() ) {
              if ( prov.detail().name() == solv.name() && _keepSpecificEditions.count( prov.detail().ed() ) ) {
                markAsKeep( solv );
              }
            }
          });
        }
      }
    }
  }

  PurgeKernels::PurgeKernels()
    : _pimpl( new Impl() )
  {

  }

  void PurgeKernels::markObsoleteKernels()
  {
    MIL << std::endl << "--------------------- Starting to mark obsolete kernels ---------------------"<<std::endl;

    if ( _pimpl->_keepSpec.empty() ) {
      WAR << "Keep spec is empty, removing nothing." << std::endl;
      return;
    }

    _pimpl->parseKeepSpec();

    if ( _pimpl->_keepRunning && !_pimpl->_detectedRunning ) {
      WAR << "Unable to detect running kernel, but keeping the running kernel was requested. Not removing any packages." << std::endl;
      return;
    }

    auto pool = ResPool::instance();
    pool.resolver().setForceResolve( true ); // set allow uninstall flag

    const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );

    // kernel flavour regex
    const str::regex kernelFlavourRegex("^kernel-(.*)$");

    // the map of all installed kernel packages, grouped by Flavour -> Arch -> Version -> (List of all packages in that category)
    // devel and source packages are grouped together
    GroupMap installedKrnlPackages;


    // packages that we plan to remove
    std::set<sat::Solvable> packagesToRemove;

    const auto addPackageToMap = [&installedKrnlPackages, &packagesToRemove] ( const GroupInfo::GroupType type, const std::string &ident, const std::string &flavour, const sat::Solvable &installedKrnlPck ) {

      if ( !installedKrnlPackages.count( ident ) )
        installedKrnlPackages.insert( std::make_pair( ident, GroupInfo(type, flavour) ) );

      auto &groupInfo = installedKrnlPackages[ ident ];
      if ( groupInfo.groupType != type || groupInfo.groupFlavour != flavour ) {
        ERR << "Got inconsistent type and flavour for ident this is a BUG: " <<  ident << std::endl
            << "Original Flavour-Type:  "<<groupInfo.groupFlavour<<"-"<<groupInfo.groupType << std::endl
            << "Competing Flavour-Type: "<< flavour << "-" << type << std::endl;
      }

      const auto currArch = installedKrnlPck.arch();
      if ( !groupInfo.archToEdMap.count( currArch ) )
        groupInfo.archToEdMap.insert( std::make_pair( currArch , EditionToSolvableMap {} ) );

      auto &editionToSolvableMap = groupInfo.archToEdMap[ currArch ];

      // calculate the "shortest" or most generic edition of all the package name provides
      // ( the key of the provides is the package name ). This generic edition is used to
      // group the packages together. This should get us around the issue that uname -r does
      // not represent the actual rpm package version anymore. ( bsc#1176740 )
      auto currCount = INT_MAX;
      Edition edToUse;
      for ( Capability prov : installedKrnlPck.provides() ) {
        if ( prov.detail().name() == installedKrnlPck.name() ) {
          if ( edToUse == Edition::noedition ) {
            edToUse = installedKrnlPck.edition();
            const auto &relStr = edToUse.release();
            currCount = std::count( relStr.begin(), relStr.end(), '.');
          } else {
            const auto &pckEd = prov.detail().ed();
            const auto &relStr = pckEd.release();
            if ( const auto pntCnt = std::count( relStr.begin(), relStr.end(), '.'); pntCnt < currCount ) {
              currCount = pntCnt;
              edToUse = pckEd;
            }
          }
        }
      }

      if ( !editionToSolvableMap.count( edToUse ) )
        editionToSolvableMap.insert( std::make_pair( edToUse, SolvableList{} ) );

      editionToSolvableMap[edToUse].push_back( installedKrnlPck );

      //in the first step we collect all packages in this list, then later we will remove the packages we want to explicitely keep
      packagesToRemove.insert( installedKrnlPck );
    };

    // the set of satSolvables that have to be kept always
    std::set<sat::Solvable> packagesToKeep;

    //collect the list of installed kernel packages
    PoolQuery q;
    q.addKind( zypp::ResKind::package );
    q.addAttribute( sat::SolvAttr::provides, "multiversion(kernel)" );
    q.setInstalledOnly();
    q.setMatchExact();

    MIL << "Searching for obsolete multiversion kernel packages." << std::endl;

    for ( sat::Solvable installedKrnlPck : q ) {

      MIL << "Found installed multiversion kernel package " << installedKrnlPck << std::endl;

      if ( installedKrnlPck.provides().matches(Capability("kernel-uname-r")) ) {
        MIL << "Identified as a kernel package " << std::endl;

        // we group kernel packages by flavour
        str::smatch what;
        str::regex_match( installedKrnlPck.name(), what, kernelFlavourRegex );
        if ( what[1].empty() ) {
          WAR << "Could not detect flavour for: " << installedKrnlPck << " ...skipping" << std::endl;
          continue;
        }

        std::string flavour = what[1];

        // XXX: No dashes in flavor names
        const auto dash = flavour.find_first_of('-');
        if ( dash != std::string::npos ) {
          flavour = flavour.substr( 0, dash );
        }

        // the ident for kernels is the flavour, to also handle cases like kernel-base and kernel which should be in the same group handled together
        addPackageToMap( GroupInfo::Kernels, flavour, flavour, installedKrnlPck );

      } else {

        // if adapting the groups do not forget to explicitely handle the group when querying the matches
        const str::regex explicitelyHandled("kernel-syms(-.*)?|kernel(-.*)?-devel");

        MIL << "Not a kernel package, inspecting more closely " << std::endl;

        // we directly handle all noarch packages that export multiversion(kernel)
        if ( installedKrnlPck.arch() == Arch_noarch ) {

          MIL << "Handling package explicitely due to architecture (noarch)."<< std::endl;
          addPackageToMap( GroupInfo::Sources, installedKrnlPck.name(), "", installedKrnlPck );

        } else if ( str::smatch match; str::regex_match( installedKrnlPck.name(), match, explicitelyHandled ) ) {

          // try to get the flavour from the name
          // if we have a kernel-syms getting no flavour means we have the "default" one, otherwise we use the flavour
          // getting no flavour for a kernel(-*)?-devel means we have the kernel-devel package otherwise the flavour specific one
          // ...yes this is horrible
          std::string flav;

          // first group match is a kernel-syms
          if ( match.size() > 1 && match[1].size() )
            flav = match[1].substr(1);
          // second group match is a kernel-flavour-devel
          else if ( match.size() > 2 &&  match[2].size() )
            flav = match[2].substr(1);
          else if ( installedKrnlPck.name() == "kernel-syms" )
            flav = "default";

          MIL << "Handling package explicitely due to name match."<< std::endl;
          addPackageToMap ( GroupInfo::RelatedBinaries, installedKrnlPck.name(), flav, installedKrnlPck );
        } else {
          MIL << "Package not explicitely handled" << std::endl;
        }
      }

    }

    MIL << "Grouped packages: " << std::endl;
    std::for_each( installedKrnlPackages.begin(), installedKrnlPackages.end(),[]( const auto &ident ){
      MIL << "\tGroup ident: "<<ident.first<<std::endl;
      MIL << "\t Group type: "<<ident.second.groupType<<std::endl;
      MIL << "\t Group flav: "<<ident.second.groupFlavour<<std::endl;
      std::for_each( ident.second.archToEdMap.begin(), ident.second.archToEdMap.end(), []( const auto &arch) {
        MIL << "\t\tArch: "<<arch.first<<std::endl;
        std::for_each( arch.second.begin(), arch.second.end(), []( const auto &edition) {
          MIL << "\t\t\tEdition: "<<edition.first<<std::endl;
          std::for_each( edition.second.begin(), edition.second.end(), []( const auto &packageId) {
            MIL << "\t\t\t\t "<<sat::Solvable(packageId)<<std::endl;
          });
        });
      });
    });

    _pimpl->fillKeepList( installedKrnlPackages, packagesToKeep, packagesToRemove );

    for ( sat::Solvable slv : packagesToRemove )
      _pimpl->removePackageAndCheck( slv, packagesToKeep, packagesToRemove );
  }

  void PurgeKernels::setUnameR( const std::string &val )
  {
    _pimpl->setUnameR( val );
  }

  std::string PurgeKernels::unameR() const
  {
    return _pimpl->_uname_r;
  }

  void PurgeKernels::setKernelArch(const Arch &arch)
  {
    _pimpl->_kernelArch = arch;
  }

  Arch PurgeKernels::kernelArch() const
  {
    return _pimpl->_kernelArch;
  }

  void PurgeKernels::setKeepSpec( const std::string &val )
  {
    _pimpl->_keepSpec = val;
  }

  std::string PurgeKernels::keepSpec() const
  {
    return _pimpl->_keepSpec;
  }

}
