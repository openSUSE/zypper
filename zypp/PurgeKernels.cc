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
#include <zypp/ui/Selectable.h>
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

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "PurgeKernels"

namespace zypp {

  struct PurgeKernels::Impl  {

    Impl() {
      struct utsname unameData;
      if ( uname( &unameData) == 0 ) {
        _kernelArch = Arch( unameData.machine );
        _uname_r = std::string( unameData.release );
      }
    }

    bool removePackageAndCheck( PoolItem &item, const str::regex &validRemovals ) const;
    void parseKeepSpec();
    void fillKeepList( const std::unordered_map< std::string, std::map< Arch, std::map<Edition, sat::Solvable> > > &installedKernels, std::set<sat::Solvable::IdType> &list ) const;
    void cleanDevelAndSrcPackages ( const str::regex &validRemovals, std::set<Edition> &validEditions, const std::string &flavour = std::string() );

    static std::string detectRunningKernel() {

      std::string kernelVersion;
      std::ifstream procKernel( "/proc/sys/kernel/osrelease" );
      if ( procKernel ) {
        std::getline( procKernel, kernelVersion );
      }
      return kernelVersion;

    }


    std::set<size_t>  _keepLatestOffsets = { 0 };
    std::set<size_t>  _keepOldestOffsets;
    std::set<Edition> _keepSpecificEditions;
    std::string       _uname_r;
    Arch              _kernelArch;
    std::string       _keepSpec = ZConfig::instance().multiversionKernels();
    bool              _keepRunning    = true;
  };

  /*!
   * tries to remove a the \ref PoolItem \a pi from the pool, solves and checks if no unexpected packages are removed due to the \a validRemovals regex.
   * If the constraint fails the changes are reverted and \a false is returned.
   */
  bool PurgeKernels::Impl::removePackageAndCheck( PoolItem &pi, const str::regex &validRemovals ) const
  {
    const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );
    auto pool = ResPool::instance();

    //remember which packages are already marked for removal, we do not need to check them again
    std::set< sat::Solvable::IdType> currentSetOfRemovals;
    for ( auto it = pool.byStatusBegin( toBeUninstalledFilter ); it != pool.byStatusEnd( toBeUninstalledFilter );  it++  )
      currentSetOfRemovals.insert( it->id() );

    pi.status().setToBeUninstalled( ResStatus::USER );

    if ( !pool.resolver().resolvePool() ) {
      MIL << "Failed to resolve pool, skipping " << pi << std::endl;
      pool.resolver().problems();
      pi.statusReset();

      return false;
    }

    for ( auto it = pool.byStatusBegin( toBeUninstalledFilter ); it != pool.byStatusEnd( toBeUninstalledFilter );  it++  ) {

      //this was set by us or marked by a previous removal, ignore them
      if ( it->status().isByUser() || (currentSetOfRemovals.find( it->id() ) != currentSetOfRemovals.end()) )
        continue;

      str::smatch what;
      if ( !str::regex_match( it->name(), what, validRemovals) ) {
        MIL << "Package " << PoolItem(*it) << " should not be removed, skipping " << pi << std::endl;
        pi.statusReset();
        return false;
      }
    }

    MIL << "Removing package: " << pi << std::endl;
    return true;
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
   * Go over the list of installed kernels and mark those as "do not remove" that match
   * a entry in the keep spec
   */
  void PurgeKernels::Impl::fillKeepList( const std::unordered_map<std::string, std::map<Arch, std::map<Edition, sat::Solvable> > > &installedKernels, std::set<sat::Solvable::IdType> &list ) const
  {
    for ( const auto &flavourMap : installedKernels ) {
      for ( const auto &archMap : flavourMap.second ) {
        size_t currOff = 0; //the current "oldest" offset ( runs from map start to end )
        size_t currROff = archMap.second.size() - 1; // the current "latest" offset ( runs from map end to start )
        for ( const auto &kernelMap : archMap.second ) {

          //if we find one of the running offsets in the keepspec, we add the kernel id the the list of packages to keep
          if (  _keepOldestOffsets.find( currOff ) != _keepOldestOffsets.end()
               || _keepLatestOffsets.find( currROff ) != _keepLatestOffsets.end()
               // a kernel might be explicitely locked by version
               || _keepSpecificEditions.find( kernelMap.second.edition() ) != _keepSpecificEditions.end() ) {
            MIL << "Marking kernel " << kernelMap.second << " as to keep." << std::endl;
            list.insert( kernelMap.second.id() ) ;
          }

          currOff++;
          currROff--;
        }
      }
    }
  }

  /*!
   * This cleans up the source and devel packages tree for a specific flavour.
   */
  void PurgeKernels::Impl::cleanDevelAndSrcPackages(const str::regex &validRemovals, std::set<Edition> &validEditions, const std::string &flavour )
  {
    bool isWithFlavour = flavour.size();

    if ( isWithFlavour )
      MIL << "Trying to remove source/devel packages for flavour " << flavour << std::endl;
    else
      MIL << "Trying to remove global/default source/devel packages "<< std::endl;

    auto withFlavour = [&isWithFlavour, &flavour]( const std::string &name ) {
      return isWithFlavour ? name+"-"+flavour : name;
    };

    //try to remove the kernel-devel-flavour and kernel-source-flavour packages
    PoolQuery q;
    q.addKind( zypp::ResKind::package );

    q.addAttribute( sat::SolvAttr::name, withFlavour("kernel-devel") );
    q.addAttribute( sat::SolvAttr::name, withFlavour("kernel-source") );
    q.setInstalledOnly();
    q.setMatchExact();

    for ( auto installedSrcPck : q ) {

      if ( validEditions.find( installedSrcPck.edition() ) == validEditions.end() ) {
        MIL << "Skipping source package " << installedSrcPck <<  " no corresponding kernel with the same version was installed." << std::endl;
        continue;
      }

      //if no package providing kernel-flavour = VERSION is installed , we are free to remove the package
      PoolQuery instKrnl;
      instKrnl.addKind( zypp::ResKind::package );
      instKrnl.setInstalledOnly();
      instKrnl.setMatchExact();
      instKrnl.addDependency( sat::SolvAttr::provides, withFlavour("kernel"), Rel::EQ, installedSrcPck.edition() );

      bool found = std::any_of ( instKrnl.begin(), instKrnl.end(), []( auto it ) {  return !PoolItem(it).status().isToBeUninstalled(); } );
      if ( found ) {
        MIL << "Skipping source package " << installedSrcPck << " binary packages with the same edition are still installed" << std::endl;
        continue;
      }

      PoolItem pi( installedSrcPck );
      removePackageAndCheck( pi, validRemovals );
    }
  }

  PurgeKernels::PurgeKernels()
    : _pimpl( new Impl() )
  {

  }

  void PurgeKernels::markObsoleteKernels()
  {
    if ( _pimpl->_keepSpec.empty() )
      return;

    _pimpl->parseKeepSpec();

    auto pool = ResPool::instance();
    pool.resolver().setForceResolve( true ); // set allow uninstall flag

    const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );

    //list of packages that are allowed to be removed automatically.
    const str::regex validRemovals("(kernel-syms(-.*)?|kgraft-patch(-.*)?|kernel-livepatch(-.*)?|.*-kmp(-.*)?)");

    //list of packages that are allowed to be removed automatically when uninstalling kernel-devel packages
    const str::regex validDevelRemovals("(kernel-source(-.*)?|(kernel-syms(-.*)?)|(kernel-devel(-.*)?)|(kernel(-.*)?-devel))");

    // kernel flavour regex
    const str::regex kernelFlavourRegex("^kernel-(.*)$");

    // the map of all installed kernels, grouped by Flavour -> Arch -> Version
    std::unordered_map< std::string, std::map< Arch, std::map<Edition, sat::Solvable> > > installedKernels;

    // the set of kernel package IDs that have to be kept always
    std::set<sat::Solvable::IdType> packagesToKeep;

    //collect the list of installed kernel packages
    PoolQuery q;
    q.addKind( zypp::ResKind::package );
    q.addAttribute( sat::SolvAttr::provides, "kernel" );
    q.setInstalledOnly();
    q.setMatchExact();

    MIL << "Searching for obsolete kernels." << std::endl;

    for ( auto installedKernel : q ) {

      MIL << "Found installed kernel " << installedKernel << std::endl;

      //we can not simply skip the running kernel to make sure the keep-spec works correctly
      if ( _pimpl->_keepRunning
           && installedKernel.provides().matches( Capability( "kernel-uname-r", Rel::EQ, Edition( _pimpl->_uname_r ) ) )
           && installedKernel.arch() == _pimpl->_kernelArch ) {
        MIL << "Marking kernel " << installedKernel << " as to keep." << std::endl;
        packagesToKeep.insert( installedKernel.id() );
      }

      str::smatch what;
      str::regex_match( installedKernel.name(), what, kernelFlavourRegex );
      if ( what[1].empty() ) {
        WAR << "Could not detect kernel flavour for: " << installedKernel << " ...skipping" << std::endl;
        continue;
      }

      const std::string flavour = what[1];
      if ( !installedKernels.count( flavour ) )
        installedKernels.insert( std::make_pair( flavour, std::map< Arch, std::map<Edition, sat::Solvable> > {} ) );

      auto &flavourMap = installedKernels[ flavour ];
      if ( !flavourMap.count( installedKernel.arch() ) )
        flavourMap.insert( std::make_pair( installedKernel.arch(), std::map<Edition, sat::Solvable>{} ) );

      flavourMap[ installedKernel.arch() ].insert( std::make_pair( installedKernel.edition(), installedKernel ) );
    }

    _pimpl->fillKeepList( installedKernels, packagesToKeep );


    MIL << "Starting to remove obsolete kernels." << std::endl;


    std::set<Edition> removedVersions;

    /*
     * If there is a KMP or livepatch depending on the package remove it as well. If
     * there is another package depending on the kernel keep the kernel. If there is
     * a package that depends on a KMP keep the KMP and a kernel required to use the
     * KMP.
     */
    for ( const auto &flavourMap : installedKernels ) {

      // collect all removed versions of this edition
      std::set<Edition> removedFlavourVersions;

      for ( const auto &archMap : flavourMap.second ) {
        for ( const auto &kernelMap : archMap.second ) {
          auto &installedKernel = kernelMap.second;

          // if the kernel is locked by the user, its not removed automatically
          if ( ui::asSelectable()( installedKernel )->hasLocks() )
            continue;

          // this package is in the keep spec, do not touch
          if ( packagesToKeep.count( installedKernel.id() ) )
            continue;

          // try to remove the kernel package, check afterwards if only expected packages have been removed
          PoolItem pi( installedKernel );
          if ( !_pimpl->removePackageAndCheck( pi, validRemovals ) ) {
            continue;
          }

          removedFlavourVersions.insert( installedKernel.edition() );

          //lets remove the kernel-flavour-devel package too
          PoolQuery develPckQ;
          develPckQ.addKind( zypp::ResKind::package );
          develPckQ.addDependency( sat::SolvAttr::name, installedKernel.name()+"-devel", Rel::EQ, installedKernel.edition() );
          develPckQ.addDependency( sat::SolvAttr::name, installedKernel.name()+"-devel-debuginfo", Rel::EQ, installedKernel.edition() );
          develPckQ.setInstalledOnly();
          develPckQ.setMatchExact();

          for ( auto krnlDevPck : develPckQ ) {

            if ( krnlDevPck.arch() != installedKernel.arch() )
              continue;

            PoolItem devPi(krnlDevPck);
            _pimpl->removePackageAndCheck( devPi, validDevelRemovals );
          }
        }
      }
      //try to remove the kernel-devel-flavour and kernel-source-flavour packages
      _pimpl->cleanDevelAndSrcPackages( validDevelRemovals, removedFlavourVersions, flavourMap.first );
      removedVersions.insert( removedFlavourVersions.begin(), removedFlavourVersions.end() );
    }

    // clean the global -devel and -source packages
    _pimpl->cleanDevelAndSrcPackages( validDevelRemovals, removedVersions );
  }

  void PurgeKernels::setUnameR( const std::string &val )
  {
    _pimpl->_uname_r = val;
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
