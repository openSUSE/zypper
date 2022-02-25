#include "list.h"
#include "main.h"
#include "repos.h"
#include "common.h"

#include "utils/flags/flagtypes.h"

using namespace zypp;

namespace {

void service_list_tr( Zypper & zypper,
                      Table & tbl,
                      const repo::RepoInfoBase_Ptr & srv,
                      unsigned reponumber,
                      const RSCommonListOptions::RSCommonListFlags & flags )
{
  ServiceInfo_Ptr service = dynamic_pointer_cast<ServiceInfo>(srv);
  RepoInfo_Ptr repo;
  if ( ! service )
    repo = dynamic_pointer_cast<RepoInfo>(srv);

  RepoGpgCheckStrings repoGpgCheck( service ? RepoGpgCheckStrings(*service) : RepoGpgCheckStrings(*repo) );

  TableRow tr( 8 );

  // number
  if ( flags & RSCommonListOptions::ServiceRepo )
  {
    if ( repo && ! repo->enabled() )
      tr << ColorString( repoGpgCheck._tagColor, "-" ).str();
    else
      tr << "";
  }
  else
    tr << ColorString( repoGpgCheck._tagColor, str::numstring(reponumber) ).str();

  // alias
  tr << ColorString( repoGpgCheck._tagColor, srv->alias() );
  // name
  tr << ColorString( repoGpgCheck._tagColor, srv->name() );
  // enabled?
  tr << repoGpgCheck._enabledYN.str();
  // GPG Check
  tr << repoGpgCheck._gpgCheckYN.str();
  // autorefresh?
  tr << repoAutorefreshStr( *srv );

  // priority
  if ( flags & RSCommonListOptions::ShowPriority )
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
  if ( flags & RSCommonListOptions::ShowURI )
  {
    if ( service )
      tr << service->url().asString();
    else
      tr << repo->url().asString();
  }

  tbl << tr;
}

}

ListServicesCmd::ListServicesCmd(std::vector<std::string> &&commandAliases_r)
  : ZypperBaseCommand (
      std::move( commandAliases_r ),
      _("services (ls) [OPTIONS]"),
      _("List all defined services."),
      _("List defined services."),
      ResetRepoManager
    )
{

}

void ListServicesCmd::printServiceList( Zypper &zypper )
{
  ServiceList services = get_all_services( zypper );

  Table tbl;

  bool with_repos = _listOptions._flags.testFlag( RSCommonListOptions::ShowWithRepos );
  //! \todo string type = zypper.cOpts().count("type");

  // header
  {
    TableHeader th;
    // fixed 'zypper services' columns
    th << "#"
       << N_("Alias")
       << N_("Name")
       << N_("Enabled")
       << N_("GPG Check")
          // translators: 'zypper repos' column - whether autorefresh is enabled for the repository
       << N_("Refresh");
    // optional columns
    if ( _listOptions._flags.testFlag( RSCommonListOptions::ShowPriority ) )
      // translators: repository priority (in zypper repos -p or -d)
      th << N_("Priority");
    th << N_("Type");
    if ( _listOptions._flags.testFlag( RSCommonListOptions::ShowURI ) )
      th << N_("URI");
    tbl << std::move(th);
  }

  bool show_enabled_only = _listOptions._flags.testFlag( RSCommonListOptions::ShowEnabledOnly );

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
      RepoManager & rm( zypper.repoManager() );

      rm.getRepositoriesInService( (*it)->alias(),
                                   make_function_output_iterator( bind( &RepoCollector::collect, &collector, _1 ) ) );

      for_( repoit, collector.repos.begin(), collector.repos.end() )
      {
        repo::RepoInfoBase_Ptr ptr( new RepoInfo(*repoit) );	// copy needed?

        if ( show_enabled_only && !repoit->enabled() )
          continue;

        if ( !servicePrinted )
        {
          service_list_tr( zypper, tbl, *it, i, _listOptions._flags );
          servicePrinted = true;
        }
        // ServiceRepo: we print repos of the current service
        service_list_tr( zypper, tbl, ptr, i, _listOptions._flags | RSCommonListOptions::ServiceRepo );
      }
    }
    if ( servicePrinted )
      continue;

    // Here: No repo enforced printing the service, so do so if
    // necessary.
    if ( show_enabled_only && !(*it)->enabled() )
      continue;

    service_list_tr( zypper, tbl, *it, i, _listOptions._flags );
  }

  if ( tbl.empty() )
    zypper.out().info( str::form(_("No services defined. Use the '%s' command to add one or more services."),
                                 "zypper addservice" ) );
  else
  {
    // sort
    if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByURI ) )
    {
      if ( _listOptions._flags.testFlag( RSCommonListOptions::ListServiceShowAll ) )
        tbl.sort( 7 );
      else if ( _listOptions._flags.testFlag( RSCommonListOptions::ShowPriority ) )
        tbl.sort( 7 );
      else
        tbl.sort( 6 );
    }
#if 0
    //BUG? This option did not exist
    else if ( zypper.cOpts().count("sort-by-alias") )
      tbl.sort( 1 );
#endif
    else if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByName ) )
      tbl.sort( 2 );
    else if ( _listOptions._flags.testFlag( RSCommonListOptions::SortByPrio) )
      tbl.sort( 5 );

    // print
    cout << tbl;
  }
}

void ListServicesCmd::printXMLServiceList( Zypper &zypper )
{
  ServiceList services = get_all_services( zypper );

  cout << "<service-list>" << endl;

  ServiceInfo_Ptr s_ptr;
  for_( it, services.begin(), services.end() )
  {
    s_ptr = dynamic_pointer_cast<ServiceInfo>(*it);
    // print also service's repos
    if ( s_ptr )
    {
      RepoCollector collector;
      RepoManager & rm( zypper.repoManager() );
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

ZyppFlags::CommandGroup ListServicesCmd::cmdOptions() const
{
  return ZyppFlags::CommandGroup();
}

void ListServicesCmd::doReset()
{
}

int ListServicesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( _listOptions._flags.testFlag( RSCommonListOptions::ShowWithRepos) )
    checkIfToRefreshPluginServices( zypper );

  if ( zypper.out().type() == Out::TYPE_XML ) {
    printXMLServiceList( zypper );
    return ZYPPER_EXIT_OK;
  }

  printServiceList( zypper );
  return ZYPPER_EXIT_OK;
}
