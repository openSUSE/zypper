#include "list.h"
#include "main.h"
#include "repos.h"
#include "common.h"

#include "utils/flags/flagtypes.h"

using namespace zypp;

namespace {

struct RepoCollector
{
  bool collect( const RepoInfo & repo )
  {
    repos.push_back( repo );
    return true;
  }
  RepoInfoList repos;
};

void service_list_tr( Zypper & zypper,
                      Table & tbl,
                      const RepoInfoBase_Ptr & srv,
                      unsigned reponumber,
                      const ListServicesCmd::ServiceListFlags & flags )
{
  ServiceInfo_Ptr service = dynamic_pointer_cast<ServiceInfo>(srv);
  RepoInfo_Ptr repo;
  if ( ! service )
    repo = dynamic_pointer_cast<RepoInfo>(srv);

  RepoGpgCheckStrings repoGpgCheck( service ? RepoGpgCheckStrings(*service) : RepoGpgCheckStrings(*repo) );

  TableRow tr( 8 );

  // number
  if ( flags & ListServicesCmd::ServiceRepo )
  {
    if ( repo && ! repo->enabled() )
      tr << ColorString( repoGpgCheck._tagColor, "-" ).str();
    else
      tr << "";
  }
  else
    tr << ColorString( repoGpgCheck._tagColor, str::numstring(reponumber) ).str();

  // alias
  tr << srv->alias();
  // name
  tr << srv->name();
  // enabled?
  tr << repoGpgCheck._enabledYN.str();
  // GPG Check
  tr << repoGpgCheck._gpgCheckYN.str();
  // autorefresh?
  tr << repoAutorefreshStr( *srv );

  // priority
  if ( flags & ListServicesCmd::ShowPriority )
  {
    if ( service )
      tr << "";
    else
      tr << repoPriorityNumber( repo->priority(), 4 );
  }

  // type
  if ( service )
    tr << service->type().asString();
  else
    tr << repo->type().asString();

  // url
  if ( flags & ListServicesCmd::ShowURI )
  {
    if ( service )
      tr << service->url().asString();
    else
      tr << repo->url().asString();
  }

  tbl << tr;
}

}

ListServicesCmd::ListServicesCmd()
  : ZypperBaseCommand (
      { "services", "ls", "service-list", "sl" },
      _("services (ls) [OPTIONS]"),
      _("List all defined services."),
      _("List defined services."),
      NoPool | ResetRepoManager
    )
{

}

void ListServicesCmd::printServiceList( Zypper &zypp_r )
{
  ServiceList services = get_all_services( zypp_r );

  Table tbl;

  bool with_repos = _flags.testFlag( ShowWithRepos );
  //! \todo string type = zypper.cOpts().count("type");

  // header
  {
    TableHeader th;
    // fixed 'zypper services' columns
    th << "#"
       << _("Alias")
       << _("Name")
       << _("Enabled")
       << _("GPG Check")
          // translators: 'zypper repos' column - whether autorefresh is enabled for the repository
       << _("Refresh");
    // optional columns
    if ( _flags.testFlag( ShowPriority ) )
      // translators: repository priority (in zypper repos -p or -d)
      th << _("Priority");
    th << _("Type");
    if ( _flags.testFlag( ShowURI ) )
      th << _("URI");
    tbl << std::move(th);
  }

  bool show_enabled_only = _flags.testFlag( ShowEnabledOnly );

  int i = 0;
  for_( it, services.begin(), services.end() )
  {
    ++i; // continuous numbering including skipped ones

    bool servicePrinted = false;
    // Unconditionally print the service before the 1st repo is
    // printed. Undesired, but possible, that a disabled service
    // owns (manually) enabled repos.
    if ( with_repos && dynamic_pointer_cast<ServiceInfo>(*it) )
    {
      RepoCollector collector;
      RepoManager & rm( zypp_r.repoManager() );

      rm.getRepositoriesInService( (*it)->alias(),
                                   make_function_output_iterator( bind( &RepoCollector::collect, &collector, _1 ) ) );

      for_( repoit, collector.repos.begin(), collector.repos.end() )
      {
        RepoInfoBase_Ptr ptr( new RepoInfo(*repoit) );	// copy needed?

        if ( show_enabled_only && !repoit->enabled() )
          continue;

        if ( !servicePrinted )
        {
          service_list_tr( zypp_r, tbl, *it, i, _flags );
          servicePrinted = true;
        }
        // ServiceRepo: we print repos of the current service
        service_list_tr( zypp_r, tbl, ptr, i, _flags | ServiceRepo );
      }
    }
    if ( servicePrinted )
      continue;

    // Here: No repo enforced printing the service, so do so if
    // necessary.
    if ( show_enabled_only && !(*it)->enabled() )
      continue;

    service_list_tr( zypp_r, tbl, *it, i, _flags );
  }

  if ( tbl.empty() )
    zypp_r.out().info( str::form(_("No services defined. Use the '%s' command to add one or more services."),
                                 "zypper addservice" ) );
  else
  {
    // sort
    if ( _flags.testFlag( SortByURI ) )
    {
      if ( _flags.testFlag( ShowAll ) )
        tbl.sort( 7 );
      else if ( _flags.testFlag( ShowPriority ) )
        tbl.sort( 7 );
      else
        tbl.sort( 6 );
    }
#if 0
    //BUG? This option did not exist
    else if ( zypper.cOpts().count("sort-by-alias") )
      tbl.sort( 1 );
#endif
    else if ( _flags.testFlag( SortByName ) )
      tbl.sort( 2 );
    else if ( _flags.testFlag( SortByPrio) )
      tbl.sort( 5 );

    // print
    cout << tbl;
  }
}

void ListServicesCmd::printXMLServiceList( Zypper &zypp_r )
{
  ServiceList services = get_all_services( zypp_r );

  cout << "<service-list>" << endl;

  ServiceInfo_Ptr s_ptr;
  for_( it, services.begin(), services.end() )
  {
    s_ptr = dynamic_pointer_cast<ServiceInfo>(*it);
    // print also service's repos
    if ( s_ptr )
    {
      RepoCollector collector;
      RepoManager & rm( zypp_r.repoManager() );
      rm.getRepositoriesInService( (*it)->alias(),
                                   make_function_output_iterator( bind( &RepoCollector::collect, &collector, _1 ) ) );
      std::ostringstream sout;
      for_( repoit, collector.repos.begin(), collector.repos.end() )
          repoit->dumpAsXmlOn( sout );
      (*it)->dumpAsXmlOn( cout, sout.str() );
      continue;
    }

    (*it)->dumpAsXmlOn( cout );
  }

  cout << "</service-list>" << endl;
}

std::vector<ZyppFlags::CommandOption> ListServicesCmd::cmdOptions() const
{
  auto that = const_cast<ListServicesCmd *>(this);;

  return {
    { "uri",  'u',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ShowURI ), _("Show also base URI of repositories.")},
    { "url", '\0',       ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BitFieldType( &that->_flags, ShowURI ), "" },
    { "priority",   'p', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ShowPriority ), _("Show also repository priority.") },
    { "details",    'd', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ShowAll      ), _("Show more information like URI, priority, type.")  },
    { "with-repos", 'r', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ShowWithRepos), _("Show also repositories belonging to the services.") },
    { "show-enabled-only", 'E', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ShowEnabledOnly ), _("Show enabled repos only.") },
    { "sort-by-uri", 'U',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ServiceListFlags( ShowURI ) | SortByURI ), _("Sort the list by URI.") },
    { "sort-by-name", 'N',      ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, SortByName ), _("Sort the list by name.") },
    { "sort-by-priority", 'P',  ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( &that->_flags, ServiceListFlags( ShowPriority ) | SortByPrio ),  _("Sort the list by repository priority.") }
  };
}

void ListServicesCmd::doReset()
{
  _flags.unsetFlag( _flags.all() );
}

int ListServicesCmd::execute( Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r )
{
  if ( _flags.testFlag( ShowWithRepos) )
    checkIfToRefreshPluginServices( zypp_r );

  if ( zypp_r.out().type() == Out::TYPE_XML ) {
    printXMLServiceList( zypp_r );
    return ZYPPER_EXIT_OK;
  }

  printServiceList( zypp_r );
  return ZYPPER_EXIT_OK;
}
