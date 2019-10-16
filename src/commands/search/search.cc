#include "search.h"
#include "src/search.h"
#include "global-settings.h"
#include "utils/flags/flagtypes.h"
#include "commands/commonflags.h"
#include "commands/commandhelpformatter.h"
#include "commands/search/search-packages-hinthack.h"

#include <zypp/base/Algorithm.h>
#include <zypp/sat/Solvable.h>
#include <zypp/Capability.h>
#include <zypp/PoolQueryResult.h>

#include <unordered_map>

namespace zypp
{
  namespace ZyppFlags
  {
    Value appendSolvAttrToSet ( std::set<zypp::sat::SolvAttr> &target_r, zypp::sat::SolvAttr value_r )
    {
      return Value (
            noDefaultValue,
            [ &target_r, value_r ] ( const CommandOption &, const boost::optional<std::string> & ) {
              target_r.insert( value_r );
            }
      );
    }

    Value setSolvAttrOptional ( boost::optional<zypp::sat::SolvAttr> &target_r, zypp::sat::SolvAttr value_r ) {
      return Value (
          noDefaultValue,
          [ &target_r, value_r ] ( const CommandOption &, const boost::optional<std::string> & ) {
            target_r = value_r;
          }
        );
    }
  }
}

namespace
{
  // search helper
  inline bool poolExpectMatchFor( const std::string & name_r, const Edition & edition_r )
  {
    for ( const auto & pi : ResPool::instance().byName( name_r ) )
    {
      if ( Edition::match( pi.edition(), edition_r ) == 0 )
	return true;
    }
    return false;
  }
}


SearchCmd::SearchCmd(std::vector<std::string> &&commandAliases_r, CmdMode cmdMode_r  ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    std::string(),
    std::string(),
    std::string(),
    ResetRepoManager
  ), _cmdMode ( cmdMode_r )
{
  if ( _cmdMode == CmdMode::Search ) {
    this->addOptionSet( _initReposOpts );
    this->addOptionSet( _notInstalledOpts );
  }
  _sortOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableNewOpt );
  _initReposOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableNewOpt );
}

void SearchCmd::setMode(const MatchMode &mode_r)
{
  _mode = mode_r;
}

void SearchCmd::addRequestedDependency(const sat::SolvAttr &dep_r)
{
  _requestedDeps.insert( dep_r );
}

std::string SearchCmd::summary() const
{
  // translators: command summary: search, se
  return _("Search for packages matching a pattern.");
}

std::vector<std::string> SearchCmd::synopsis() const
{
  if ( _cmdMode == CmdMode::Search ) {
    return {
      // translators: command synopsis; do not translate lowercase words
      _("search (se) [OPTIONS] [QUERYSTRING] ...")
    };
  }
  return {
    // translators: command synopsis; do not translate lowercase words
    _("patch-search [OPTIONS] [QUERYSTRING...]")
  };
}

std::string SearchCmd::description() const
{
  if ( _cmdMode == CmdMode::Search ) {
    return (
      // translators: command description
      std::string( _("Search for packages matching any of the given search strings.") ) + "\n\n"
      // translators: command description
      + _("* and ? wildcards can also be used within search strings. If a search string is enclosed in '/', it's interpreted as a regular expression.")
    );
  }

  // translators: command description
  return _("Search for patches matching given search strings.");
}

zypp::ZyppFlags::CommandGroup SearchCmd::cmdOptions() const
{

  auto &that = *const_cast<SearchCmd *>(this);
  zypp::ZyppFlags::CommandGroup grp = {
    {
      { "match-substrings", 0, ZyppFlags::NoArgument, ZyppFlags::WriteFixedValueType( that._mode, MatchMode::Substrings ),
            // translators: --match-substrings
            _("Search for a match to partial words (default).")
      },
      { "match-words", 0, ZyppFlags::NoArgument, ZyppFlags::WriteFixedValueType( that._mode, MatchMode::Words ),
            // translators: --match-words
            _("Search for a match to whole words only.")
      },
      { "match-exact", 'x', ZyppFlags::NoArgument, ZyppFlags::WriteFixedValueType( that._mode, MatchMode::Exact ),
            // translators: -x, --match-exact
            _("Searches for an exact match of the search strings.")
      }
    },{
      { "match-substrings", "match-words", "match-exact" }
    }
  };

  if ( _cmdMode == CmdMode::Search ) {
    grp.options.insert( grp.options.end(),
    {
        { "provides", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::provides ),
              // translators: --provides
              _("Search for packages which provide the search strings.")
        },
        { "requires", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::requires ),
              // translators: --requires
              _("Search for packages which require the search strings.")
        },
        { "recommends", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::recommends ),
              // translators: --recommends
              _("Search for packages which recommend the search strings.")
        },
        { "supplements", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::supplements ),
              // translators: --supplements
              _("Search for packages which supplement the search strings.")
        },
        { "conflicts", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::conflicts ),
              // translators: --conflicts
              _("Search packages conflicting with search strings.")
        },
        { "obsoletes", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::obsoletes ),
              // translators: --obsoletes
              _("Search for packages which obsolete the search strings.")
        },
        { "suggests", '\0', ZyppFlags::NoArgument, ZyppFlags::appendSolvAttrToSet( that._requestedDeps, sat::SolvAttr::suggests ),
              // translators: --suggests
              _("Search for packages which suggest the search strings.")
        },
        { "provides-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::provides ),
          // translators: --provides-pkg
          _("Search for all packages that provide any of the provides of the package(s) matched by the input parameters.")
        },
        { "requires-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::requires ),
          // translators: --requires-pkg
          _("Search for all packages that require any of the provides of the package(s) matched by the input parameters.")
        },
        { "recommends-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::recommends ),
          // translators: --recommends-pkg
          _("Search for all packages that recommend any of the provides of the package(s) matched by the input parameters.")
        },
        { "supplements-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::supplements ),
          // translators: --supplements-pkg
          _("Search for all packages that supplement any of the provides of the package(s) matched by the input parameters.")
        },
        { "conflicts-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::conflicts ),
          // translators: --conflicts-pkg
          _("Search for all packages that conflict with any of the package(s) matched by the input parameters.")
        },
        { "obsoletes-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::obsoletes ),
          // translators: --obsoletes-pkg
          _("Search for all packages that obsolete any of the package(s) matched by the input parameters.")
        },
        { "suggests-pkg", '\0', ZyppFlags::NoArgument, ZyppFlags::setSolvAttrOptional( that._requestedReverseSearch, sat::SolvAttr::suggests ),
          // translators: --suggests-pkg
          _("Search for all packages that suggest any of the provides of the package(s) matched by the input parameters.")
        },
        CommonFlags::resKindSetFlag( that._requestedTypes,
              // translators: -t, --type <TYPE>
              _("Search only for packages of the specified type.")
        ),
        { "name", 'n', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._forceNameAttr, ZyppFlags::StoreTrue ),
              // translators: -n, --name
             _("Useful together with dependency options, otherwise searching in package name is default.")
        },
        { "file-list", 'f', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._searchFileList, ZyppFlags::StoreTrue, _searchFileList ),
              // translators: -f, --file-list
              _("Search for a match in the file list of packages.")
        }
    } );

    grp.conflictingOptions.push_back( { "provides-pkg", "requires-pkg", "recommends-pkg", "supplements-pkg", "conflicts-pkg", "obsoletes-pkg", "suggests-pkg"  } );
  }

  grp.options.insert( grp.options.end(),
  {
      { "search-descriptions", 'd', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._searchDesc, ZyppFlags::StoreTrue, _searchDesc ),
            // translators: -d, --search-descriptions
            _("Search also in package summaries and descriptions.")
      },
      {"case-sensitive", 'C', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._caseSensitive, ZyppFlags::StoreTrue, _caseSensitive ),
            // translators: -C, --case-sensitive
            _("Perform case-sensitive search.")
      }
  });

  if ( _cmdMode == CmdMode::Search ) {
    grp.options.insert( grp.options.end(),
    {
      CommonFlags::detailsFlag( that._details, 's',
            // translators: -s, --details
            _("Show each available version in each repository on a separate line.")
      ),
      {"verbose", 'v', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._verbose, ZyppFlags::StoreTrue, _caseSensitive ),
            // translators: -v, --verbose
            _("Like --details, with additional information where the search has matched (useful for search in dependencies).")
      }
    });
  }

  return grp;
}

std::string SearchCmd::help()
{
  if ( _cmdMode == CmdMode::Search )
    return ZypperBaseCommand::help();

  CommandHelpFormater cliHlp;
  for ( const std::string &syn : synopsis() )
    cliHlp.synopsis( syn );

  cliHlp.description( description())
      .descriptionAliasCmd( "zypper -r search -t patch --detail ..." );
  return cliHlp;
}

void SearchCmd::doReset()
{
  _mode = MatchMode::Default;
  _forceNameAttr = false;
  _searchFileList = false;
  _searchDesc = false;
  _caseSensitive = false;
  _details = false;
  _verbose = false;
  _requestedDeps.clear();
  _requestedTypes.clear();
}

int SearchCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // check args...
  PoolQuery query;
  TriBool inst_notinst = indeterminate;

  if ( zypper.config().disable_system_resolvables || _notInstalledOpts._mode == SolvableFilterMode::ShowOnlyNotInstalled )
  {
    query.setUninstalledOnly(); // beware: this is not all to it, look at zypper-search, _only_not_installed
    inst_notinst = false;
  }

  if ( _notInstalledOpts._mode == SolvableFilterMode::ShowOnlyInstalled ) {
    inst_notinst = true;
    zypper.configNoConst().no_refresh = true;
    //  query.setInstalledOnly();
  }

  switch ( _mode  ) {
    case MatchMode::Substrings:
      query.setMatchSubstring();	// this is also the PoolQuery default
      break;
    case MatchMode::Words:
      query.setMatchWord();
      break;
    case MatchMode::Exact:
      query.setMatchExact();
      break;
    case MatchMode::Default:
    break;
  }

  if ( _caseSensitive )
    query.setCaseSensitive();

  if ( _cmdMode == CmdMode::RugPatchSearch )
    query.addKind( ResKind::patch );
  else if ( _requestedTypes.size() > 0 )
  {
    for ( const ResKind &knd : _requestedTypes )
      query.addKind( knd );
  }

  // load system data...
  int code = defaultSystemSetup(  zypper, InitTarget | InitRepos | LoadResolvables | Resolve  );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  // build query...

  // add available repos to query
  if ( InitRepoSettings::instance()._repoFilter.size() )
  {
    auto &rData = zypper.runtimeData();
    for_(repo_it, rData.repos.begin(), rData.repos.end() )
    {
      query.addRepo( repo_it->alias() );
      if ( !repo_it->enabled() )
      {
        zypper.out().warning( str::Format(_("Specified repository '%s' is disabled.")) % repo_it->asUserString() );
      }
    }
  }

  // make sure we search by name if the user explicitely forced it or if no deps where requested
  if ( _requestedDeps.empty() || _forceNameAttr )
    _requestedDeps.insert( sat::SolvAttr::name );

  bool details = _details || _verbose;
  // add argument strings and attributes to query
  for_( it, positionalArgs_r.begin(), positionalArgs_r.end() )
  {
    Capability cap( *it );
    std::string name = cap.detail().name().asString();

    // bsc#1119873 zypper search: inconsistent results for `-t package kernel-default` vs `package:kernel-default`
    // Capability parser strips 'package:' prefix from name, because ident for package and srcpackage does not contain
    // the prefix but instead is only differentiated by arch.
    // We need to add package prefix again, the srcpackage is correctly matched since the srcpackage: prefix is not stripped
    ResKind explicitBuildin = ResKind::explicitBuiltin( *it );
    if ( explicitBuildin == ResKind::package )
      name = explicitBuildin.asString() + ":" + name;

    if ( cap.detail().isVersioned() )
      details = true;	// show details if any search string includes an edition

    // Default Match::OTHER indicates to merge name into the global search string and mode.
    Match::Mode matchmode = Match::OTHER;
    if ( _mode == MatchMode::Default )
    {
      if ( name.size() >= 2 && *name.begin() == '/' && *name.rbegin() == '/' )
      {
        name = name.substr( 1, name.size()-2 );
        matchmode = Match::REGEX;
      }
      else if ( name.find_first_of("?*") != std::string::npos )
        matchmode = Match::GLOB;
    }
    // else: match mode explicitly requested by cli arg

    // NOTE: We use the  addDependency  overload taking a  matchmode  argument for ALL
    // kinds of attributes, not only for dependencies. A constraint on 'op version'
    // will automatically be applied to match a matching dependency or to match
    // the matching solvables version, depending on the kind of attribute.
    for ( const zypp::sat::SolvAttr &attr : _requestedDeps ) {

      //add the basic dependency
      query.addDependency( attr , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );

      //handle special cases
      if ( attr == sat::SolvAttr::provides && str::regex_match( name.c_str(), std::string("^/") ) ) {
        // in case of path names also search in file list
        query.setFilesMatchFullPath( true );
        query.addDependency( sat::SolvAttr::filelist , name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );

      } else if ( attr == sat::SolvAttr::filelist ) {

        query.setFilesMatchFullPath( true );

      } else if ( attr == sat::SolvAttr::name ) {

        if ( matchmode == Match::OTHER && cap.detail().isNamed() )
        {
          // ARG did not require a specific matchmode.
          // Handle "N-V" and "N-V-R" cases. Name must match exact,
          // Version/Release must not be empty. If versioned matches are
          // found, don't forget to show details.
          std::string::size_type pos = name.find_last_of( "-" );
          if ( pos != std::string::npos && pos != 0 && pos != name.size()-1 )
          {
            std::string n( name.substr(0,pos) );
            std::string r( name.substr(pos+1) );
            Edition e( r );
            query.addDependency( sat::SolvAttr::name, n, Rel::EQ, e, Arch(cap.detail().arch()), Match::STRING );
            if ( poolExpectMatchFor( n, e ) )
              details = true;	// show details if any search string includes an edition

            std::string::size_type pos2 = name.find_last_of( "-", pos-1 );
            if ( pos2 != std::string::npos && pos2 != 0 &&  pos2 != pos-1)
            {
              n = name.substr(0,pos2);
              e = Edition( name.substr(pos2+1,pos-pos2-1), r );
              query.addDependency( sat::SolvAttr::name, n, Rel::EQ, e, Arch(cap.detail().arch()), Match::STRING );
              if ( poolExpectMatchFor( n, e ) )
                details = true;	// show details if any search string includes an edition
            }
          }
        }
      }
    }

    if ( _searchDesc )
    {
      query.addDependency( sat::SolvAttr::summary, name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
      query.addDependency( sat::SolvAttr::description, name, cap.detail().op(), cap.detail().ed(), Arch(cap.detail().arch()), matchmode );
    }
  }

  Table t;
  try
  {
    if ( _requestedReverseSearch.is_initialized() ) {

      std::unordered_map< sat::Solvable, CapabilitySet > matchedSolvables;
      const auto reqSearchAttrib = _requestedReverseSearch.get();

      for ( const auto slv : query ) {

        bool isInstalled = slv.isSystem();
        if ( isInstalled && _notInstalledOpts._mode == SolvableFilterMode::ShowOnlyNotInstalled )
          continue;
        if ( !isInstalled && _notInstalledOpts._mode == SolvableFilterMode::ShowOnlyInstalled )
          continue;

        sat::Queue q = sat::Pool::instance().whatMatchesSolvable( reqSearchAttrib, slv  );

        for ( auto matchedSolvId : q ) {

          sat::Solvable matchedSolv ( static_cast<sat::Solvable::IdType>(matchedSolvId) );
          auto p = matchedSolvables.insert( make_pair( std::move(matchedSolv), CapabilitySet()) );

          if ( _verbose ) {
            CapabilitySet matchedCaps = matchedSolv.matchesSolvable( reqSearchAttrib, slv).second;
            p.first->second.insert( matchedCaps.begin(), matchedCaps.end() );
          }
        }
      }

      if ( details ) {
        FillSearchTableSolvable callback( t, inst_notinst );
        std::for_each( matchedSolvables.begin(), matchedSolvables.end(), [&callback, verb = _verbose, &reqSearchAttrib ]( auto elem ){
          if ( verb )
            callback( elem.first, reqSearchAttrib, elem.second );
          else
            callback( elem.first, reqSearchAttrib, {} );
        } );
      } else {

        PoolQueryResult res;
        std::for_each( matchedSolvables.begin(), matchedSolvables.end(), [ &res ]( const auto &v ){ res+=v.first; } );

        FillSearchTableSelectable callback( t, inst_notinst );
        std::for_each( res.selectableBegin(), res.selectableEnd(), callback);
      }

    } else {
      if ( _cmdMode == CmdMode::RugPatchSearch )
      {
        FillPatchesTable callback( t, inst_notinst );
        invokeOnEach( query.poolItemBegin(), query.poolItemEnd(), callback );
      }
      else if ( details )
      {
        FillSearchTableSolvable callback( t, inst_notinst );
        if ( _verbose )
        {
          // Option 'verbose' shows where (e.g. in 'requires', 'name') the search has matched.
          // Info is available from PoolQuery::const_iterator.
          for_( it, query.begin(), query.end() )
            callback( it );
        }
        else
        {
          for ( const auto slv : query )
            callback( slv );
        }
      }
      else
      {
        FillSearchTableSelectable callback( t, inst_notinst );
        invokeOnEach( query.selectableBegin(), query.selectableEnd(), callback );
      }
    }

    if ( t.empty() )
    {
      // translators: empty search result message
      zypper.out().info(_("No matching items found."), Out::QUIET );
      zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    }
    else
    {
      cout << endl; //! \todo  out().separator()?

      if ( _cmdMode == CmdMode::RugPatchSearch )
      {
        if ( _sortOpts._mode == SortResultOptionSet::ByRepo )
          t.sort( { 0, 1, Table::UserData } );
        else
          t.sort( { 1, Table::UserData } ); // sort by name
      }
      else if ( _details )
      {
        if ( _sortOpts._mode == SortResultOptionSet::ByRepo )
          t.sort( { 5, 1, Table::UserData } );
        else
          t.sort( { 1, Table::UserData } ); // sort by name
      }
      else
      {
        // sort by name (can't sort by repo)
        t.sort( 1 );
        if ( !zypper.config().no_abbrev )
          t.allowAbbrev( 2 );
      }

      //cout << t; //! \todo out().table()?
      zypper.out().searchResult( t );
    }

    if ( !_requestedReverseSearch.is_initialized() )
      searchPackagesHintHack::callOrNotify( zypper );

  } catch ( const Exception & e )  {
    zypper.out().error( e, _("Problem occurred initializing or executing the search query") + std::string(":"),
      std::string(_("See the above message for a hint.")) + " "
        + _("Running 'zypper refresh' as root might resolve the problem.") );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
  }

  return zypper.exitCode();
}
