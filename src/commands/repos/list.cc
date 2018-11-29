#include "list.h"
#include "Zypper.h"
#include "repos.h"
#include "utils/messages.h"
#include "utils/flags/flagtypes.h"

#include <zypp/RepoManager.h>

#include <fstream>

using namespace zypp;

namespace {

void print_repos_to( const std::list<RepoInfo> & repos, std::ostream & out )
{
  for_( it, repos.begin(), repos.end() )
    it->dumpAsIniOn( out ) << endl;
}

/** Repo list as xml */
void print_xml_repo_list( Zypper & zypper, std::list<RepoInfo> repos )
{
  cout << "<repo-list>" << endl;
  for_( it, repos.begin(), repos.end() )
    it->dumpAsXmlOn( cout );
  cout << "</repo-list>" << endl;
}

void print_repo_details( Zypper & zypper, std::list<RepoInfo> & repos )
{
  bool first = true;
  for_( it, repos.begin(), repos.end() )
  {
    const RepoInfo & repo( *it );

    PropertyTable p;
    RepoGpgCheckStrings repoGpgCheck( repo );

    p.add( _("Alias"),		repo.alias() );
    p.add( _("Name"),		repo.name() );
    p.add( _("URI"),		(repo.baseUrlSet()
				 ? repo.url().asString()
				 : (repo.mirrorListUrl().asString().empty()
				    ? "n/a"
				    : repo.mirrorListUrl().asString())) );
    p.add( _("Enabled"),	repoGpgCheck._enabledYN.str() );
    p.add( _("GPG Check"),	repoGpgCheck._gpgCheckYN.str() );
    p.add( _("Priority"),	repoPriorityNumberAnnotated( repo.priority() ) );
    p.add( _("Autorefresh"),	(repo.autorefresh() ? _("On") : _("Off")) );
    p.add( _("Keep Packages"),	(repo.keepPackages() ? _("On") : _("Off")) );
    p.add( _("Type"),		repo.type().asString() );
    p.add( _("GPG Key URI"),	repo.gpgKeyUrl() );
    p.add( _("Path Prefix"),	repo.path() );
    p.add( _("Parent Service"),	repo.service() );
    p.lst( _("Keywords"),	repo.contentKeywords() );
    p.add( _("Repo Info Path"),	repo.filepath() );
    p.add( _("MD Cache Path"),	repo.metadataPath() );


    if ( first )
      first = false;
    else
      cout << endl;
    cout << p;
  }
}

}

ListReposCmd::ListReposCmd(const std::vector<std::string> &commandAliases_r)
  : ZypperBaseCommand(
      commandAliases_r,
      _("repos (lr) [OPTIONS] [REPO] ..."),
      _("List all defined repositories."),
      _("List all defined repositories."),
      ResetRepoManager
     )
{ }

zypp::ZyppFlags::CommandGroup ListReposCmd::cmdOptions() const
{
  auto that = const_cast<ListReposCmd *>(this);
  return {{
      { "export", 'e', ZyppFlags::RequiredArgument, ZyppFlags::StringType(&that->_exportFile, boost::optional<const char *>(), "FILE.repo"),
            // translators: -e, --export <FILE.repo>
            _("Export all defined repositories as a single local .repo file.") }
  }};
}

void ListReposCmd::doReset()
{
  _exportFile.clear();
}

int ListReposCmd::execute( Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r )
{
  checkIfToRefreshPluginServices( zypp_r );

  RepoManager & manager = zypp_r.repoManager();
  RuntimeData & gData = zypp_r.runtimeData();
  std::list<RepoInfo> repos;
  std::list<std::string> not_found;

  try
  {
    if ( positionalArgs_r.empty() )
      repos.insert( repos.end(), manager.repoBegin(), manager.repoEnd() );
    else
    {
      get_repos( zypp_r, positionalArgs_r.begin(),positionalArgs_r.end(), repos, not_found );
      report_unknown_repos( zypp_r.out(), not_found );
    }
  }
  catch ( const Exception & e )
  {
    ZYPP_CAUGHT( e );
    zypp_r.out().error( e, _("Error reading repositories:") );
    return ( ZYPPER_EXIT_ERR_ZYPP );
  }

  // add the temporary repos specified with the --plus-repo to the list
  if ( !gData.temporary_repos.empty() )
    repos.insert( repos.end(), gData.temporary_repos.begin(), gData.temporary_repos.end() );

  // export to file or stdout in repo file format
  /// \todo dedup writing code in list_services
  if ( !_exportFile.empty() )
  {
    std::string filename_str = _exportFile;
    if ( filename_str == "-" )
    {
      print_repos_to(repos, cout);
    }
    else
    {
      if ( filename_str.rfind(".repo") == std::string::npos )
        filename_str += ".repo";

      Pathname file( filename_str );
      std::ofstream stream( file.c_str() );
      if ( !stream )
      {
        zypp_r.out().error( str::Format(_("Can't open %s for writing.")) % file.asString(),
			    _("Maybe you do not have write permissions?") );
        return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
      }
      else
      {
        print_repos_to( repos, stream );
        zypp_r.out().info( str::Format(_("Repositories have been successfully exported to %s.")) % file,
			   Out::QUIET );
      }
    }
  }
  // print repo list as xml
  else if ( zypp_r.out().type() == Out::TYPE_XML )
    print_xml_repo_list( zypp_r, repos );
  else if ( !positionalArgs_r.empty() )
    print_repo_details( zypp_r, repos );
  // print repo list as table
  else
    printRepoList( zypp_r, repos );

  if ( !not_found.empty() ) {
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  } else if ( repos.empty() ) {
    return ( ZYPPER_EXIT_NO_REPOS );
  }

  return ZYPPER_EXIT_OK;
}

void ListReposCmd::printRepoList( Zypper & zypper, const std::list<RepoInfo> & repos )
{
  Table tbl;
  bool all = _listOptions._flags.testFlag( RSCommonListOptions::ListRepoShowAll );
  std::string list_cols = zypper.config().repo_list_columns;

  bool showalias = _listOptions._flags.testFlag( RSCommonListOptions::ShowAlias )
      || list_cols.find_first_of("aA") != std::string::npos;
  bool showname = _listOptions._flags.testFlag( RSCommonListOptions::ShowName )
      || list_cols.find_first_of("nN") != std::string::npos;
  bool showrefresh = _listOptions._flags.testFlag( RSCommonListOptions::ShowRefresh )
      || list_cols.find_first_of("rR") != std::string::npos;
  bool showuri = _listOptions._flags.testFlag( RSCommonListOptions::ShowURI )
      || list_cols.find_first_of("uU") != std::string::npos;
  bool showprio = _listOptions._flags.testFlag( RSCommonListOptions::ShowPriority )
      || list_cols.find_first_of("pP") != std::string::npos;
  bool showservice = _listOptions._flags.testFlag( RSCommonListOptions::ShowWithService );
  bool sort_override = _listOptions._flags.testFlag( RSCommonListOptions::SortByURI )
      || _listOptions._flags.testFlag( RSCommonListOptions::SortByPrio )
      || _listOptions._flags.testFlag( RSCommonListOptions::SortByAlias )
      || _listOptions._flags.testFlag( RSCommonListOptions::SortByName );
  bool show_enabled_only = _listOptions._flags.testFlag( RSCommonListOptions::ShowEnabledOnly );


  // header
  TableHeader th;
  // keep count of columns so that we know which one to sort
  // TODO might be worth to improve Table to allow named columns so this can be avoided
  unsigned index = 0;
  // number of the column to sort by
  unsigned sort_index = 0;

  // repo number
  th << "#";

  // alias
  if ( all || showalias )
  {
    th << _("Alias");
    ++index;
    // if (zypper.cOpts().count("sort-by-alias")
    //    || (list_cols.find("A") != std::string::npos && !sort_override))
    // sort by alias by default
    sort_index = index;
  }

  // name
  if ( all || showname )
  {
     th << _("Name");
     ++index;
     if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByName ) || ( list_cols.find("N") != std::string::npos && !sort_override ) )
       sort_index = index;
  }

  // 'enabled' flag
  th << _("Enabled");
  ++index;

  // GPG Check
  th << _("GPG Check");
  ++index;

  // 'autorefresh' flag
  if ( all || showrefresh )
  {
    // translators: 'zypper repos' column - whether autorefresh is enabled
    // for the repository
    th << _("Refresh");
    ++index;
    if ( list_cols.find("R") != std::string::npos && !sort_override )
      sort_index = index;
  }

  // priority
  if ( all || showprio )
  {
    // translators: repository priority (in zypper repos -p or -d)
    th << _("Priority");
    ++index;
    if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByPrio ) || ( list_cols.find("P") != std::string::npos && !sort_override ) )
      sort_index = Table::UserData;
  }

  // type
  if ( all )
  {
    th << _("Type");
    ++index;
  }

  // URI
  if ( all || showuri )
  {
    th << _("URI");
    ++index;
    if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByURI ) || ( list_cols.find("U") != std::string::npos && !sort_override ) )
      sort_index = index;
  }

  // service alias
  if ( all || showservice )
  {
    th << _("Service");
    ++index;
  }

  tbl << th;

  // table data
  int i = 0;
  unsigned nindent = repos.size() > 9 ? repos.size() > 99 ? 3 : 2 : 1;
  for_( it, repos.begin(), repos.end() )
  {
    ++i; // continuous numbering including skipped ones
    RepoInfo repo = *it;

    if ( show_enabled_only && !repo.enabled() )
      continue;

    TableRow tr( index );
    RepoGpgCheckStrings repoGpgCheck( repo );	// color strings for tag/enabled/gpgcheck

    // number
    tr << ColorString( repoGpgCheck._tagColor, str::numstring( i, nindent ) ).str();
    // alias
    if ( all || showalias ) tr << repo.alias();
    // name
    if ( all || showname ) tr << repo.name();
    // enabled?
    tr << repoGpgCheck._enabledYN.str();
    // GPG Check
    tr << repoGpgCheck._gpgCheckYN.str();
    // autorefresh?
    if ( all || showrefresh )
      tr << repoAutorefreshStr( repo );
    // priority
    if ( all || showprio )
      // output flush right; use custom sort index as coloring will break lex. sort
      ( tr << repoPriorityNumber( repo.priority(), 4 ) ).userData( repo.priority() );
    // type
    if ( all )
      tr << repo.type().asString();
    // url
    /**
     * \todo properly handle multiple baseurls - show "(multiple)"
     */
    if ( all || showuri )
      tr << (repo.baseUrlSet() ? repo.url().asString() : (repo.mirrorListUrl().asString().empty() ? "n/a" : repo.mirrorListUrl().asString()) );

    if ( all || showservice )
      tr << repo.service();

    tbl << tr;
  }

  if ( tbl.empty() )
  {
    zypper.out().warning(_("No repositories defined.") );
    zypper.out().info(_("Use the 'zypper addrepo' command to add one or more repositories.") );
  }
  else
  {
    if ( !showprio )
    {
      repoPrioSummary( zypper );
      zypper.out().gap();
    }
    // sort
    tbl.sort( sort_index );
    // print
    cout << tbl;
  }
}

