#include <iostream> // for xml and table output
#include <sstream>
#include <boost/format.hpp>

#include <zypp/base/LogTools.h>
#include <zypp/ZYppFactory.h>
#include <zypp/base/Algorithm.h>
#include <zypp/PoolQuery.h>

#include <zypp/Patch.h>

#include "SolverRequester.h"
#include "Table.h"
#include "update.h"
#include "main.h"

namespace std
{
  // std::container stream output
  using zypp::operator<<;
}
using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;

typedef set<PoolItem> Candidates;

static void
find_updates( const ResKindSet & kinds, Candidates & candidates );

///////////////////////////////////////////////////////////////////
/// \class Issues
/// \brief An issue (Type,Id) pair
///////////////////////////////////////////////////////////////////
struct Issue : std::pair<std::string, std::string>
{
  Issue( std::string issueType_r, std::string issueId_r )
  : std::pair<std::string, std::string>( std::move(issueType_r), std::move(issueId_r) )
  {}

  std::string &       type()			{ return first; }
  const std::string & type()		const	{ return first; }
  bool                anyType()		const	{ return type().empty(); }
  bool                specificType()	const	{ return !anyType(); }

  std::string &       id()			{ return second; }
  const std::string & id()		const	{ return second; }
  bool                anyId()		const 	{ return id().empty(); }
  bool                specificId()	const	{ return !anyId(); }
};

///////////////////////////////////////////////////////////////////
/// \class CliScanIssues
/// \brief Setup issue (Type,Id) pairs from CLI
///////////////////////////////////////////////////////////////////
struct CliScanIssues : public std::set<Issue>
{
  CliScanIssues()
  {
    checkCLI( "issues",   ""/*any*/ );
    checkCLI( "bugzilla", "bugzilla" );
    checkCLI( "bz",       "bugzilla" );
    checkCLI( "cve",      "cve" );
  }

private:
  void checkCLI( const std::string & cliOption_r, const std::string & issueType_r )
  {
    Zypper & zypper( *Zypper::instance() );

    bool anyId = false;	// plain option without opt. args
    std::vector<std::string> issueIds;

    for ( const auto & val : zypper.cOptValues( cliOption_r ) )
    {
      if ( str::split( val, std::back_inserter(issueIds), "," ) == 0 )
      {	anyId = true; }
    }

    if ( issueIds.empty() )
    {
      if ( anyId )
      { insert( value_type( issueType_r, std::string() ) ); }
    }
    else
    {
      if ( anyId )
      {
	zypper.out().warning(str::form(
	  _("Ignoring %s without argument because similar option with an argument has been specified."),
	  ("--" + cliOption_r).c_str() ));
      }
      for ( auto & val : issueIds )
      { insert( value_type( issueType_r, std::move(val) ) ); }
    }
  }
};

// ----------------------------------------------------------------------------
//
// Updates
//
// The following scenarios are handled distinctly:
// * -t patch (default), no arguments
// * -t package, no arguments
//   - uses Resolver::doUpdate()
// * -t {other}, no arguments
// * -t patch foo
// * -t package foo
//   - addRequires(>installed-version) if available
// * -t {other} foo
//   - addRequires(>installed-version) if available
//
// update summary must correspond to list-updates and patch-check
// ----------------------------------------------------------------------------

void patch_check ()
{
  Out & out = Zypper::instance()->out();
  RuntimeData & gData = Zypper::instance()->runtimeData();
  DBG << "patch check" << endl;
  gData.patches_count = gData.security_patches_count = 0;

  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::patch),
    e = God->pool().byKindEnd(ResKind::patch);
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();
    Patch::constPtr patch = asKind<Patch>(res);

    if (it->isRelevant() && !it->isSatisfied())
    {
      gData.patches_count++;
      if (patch->categoryEnum() == Patch::CAT_SECURITY)
        gData.security_patches_count++;
    }
  }

  ostringstream s;
  // translators: %d is the number of needed patches
  s << format(_PL("%d patch needed", "%d patches needed", gData.patches_count))
      % gData.patches_count
    << " ("
    // translators: %d is the number of security patches
    << format(_PL("%d security patch", "%d security patches", gData.security_patches_count))
      % gData.security_patches_count
    << ")";
  out.info(s.str(), Out::QUIET);
}

// ----------------------------------------------------------------------------
inline const char *const patchStatusAsString( const PoolItem & pi_r )
{
  switch ( pi_r.status().validate() )
  {
    case zypp::ResStatus::BROKEN:	return pi_r.isUnwanted() ? "unwanted"
								 : "needed";	break;
    case zypp::ResStatus::SATISFIED:	return "applied";	break;
    case zypp::ResStatus::NONRELEVANT:	return "not-needed";	break;

    case zypp::ResStatus::UNDETERMINED:	// fall through
    default:
      break;
  }
  return "undetermined";
}

// returns true if restartSuggested() patches are availble
static bool xml_list_patches (Zypper & zypper)
{
  const zypp::ResPool& pool = God->pool();

  unsigned int patchcount=0;
  bool pkg_mgr_available = false;
  Patch::constPtr patch;

  ResPool::byKind_iterator
    it = pool.byKindBegin(ResKind::patch),
    e  = pool.byKindEnd(ResKind::patch);

  // check whether there are packages affecting the update stack
  for (; it != e; ++it)
  {
    patch = asKind<Patch>(it->resolvable());
    if (it->isRelevant() && !it->isSatisfied() && patch->restartSuggested())
    {
      pkg_mgr_available = true;
      break;
    }
  }

  it = pool.byKindBegin(ResKind::patch);
  for (; it != e; ++it, ++patchcount)
  {
    if (zypper.cOpts().count("all") || it->isBroken())
    {
      ResObject::constPtr res = it->resolvable();
      Patch::constPtr patch = asKind<Patch>(res);

      // if updates stack patches are available, show only those
      if ((pkg_mgr_available && patch->restartSuggested()) || !pkg_mgr_available)
      {
        cout << " <update ";
        cout << "name=\"" << res->name () << "\" ";
        cout << "edition=\""  << res->edition ().asString() << "\" ";
	cout << "arch=\""  << res->arch().asString() << "\" ";
	cout << "status=\""  << patchStatusAsString( *it ) << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "severity=\"" <<  patch->severity() << "\" ";
        cout << "pkgmanager=\"" << (patch->restartSuggested() ? "true" : "false") << "\" ";
        cout << "restart=\"" << (patch->rebootSuggested() ? "true" : "false") << "\" ";

        Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
        if (zypper.globalOpts().reboot_req_non_interactive)
          ignoreFlags |= Patch::Reboot;
	if ( zypper.cOpts().count("auto-agree-with-licenses") || zypper.cOpts().count("agree-to-third-party-licenses") )
	  ignoreFlags |= Patch::License;

        cout << "interactive=\"" << (patch->interactiveWhenIgnoring(ignoreFlags) ? "true" : "false") << "\" ";
        cout << "kind=\"patch\"";
        cout << ">" << endl;
        cout << "  <summary>" << xml::escape(patch->summary()) << "  </summary>" << endl;
        cout << "  <description>" << xml::escape(patch->description()) << "</description>" << endl;
        cout << "  <license>" << xml::escape(patch->licenseToConfirm()) << "</license>" << endl;

        if ( !patch->repoInfo().alias().empty() )
        {
          cout << "  <source url=\"" << xml::escape(patch->repoInfo().url().asString());
          cout << "\" alias=\"" << xml::escape(patch->repoInfo().alias()) << "\"/>" << endl;
        }

        cout << " </update>" << endl;
      }
    }
  }

  //! \todo change this from appletinfo to something general, define in xmlout.rnc
  if (patchcount == 0)
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void xml_list_updates(const ResKindSet & kinds)
{
  Candidates candidates;
  find_updates (kinds, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "arch=\""  << res->arch().asString() << "\" ";
    cout << "kind=\"" << res->kind() << "\" ";
    cout << ">" << endl;
    cout << "  <summary>" << xml::escape(res->summary()) << "  </summary>" << endl;
    cout << "  <description>" << xml::escape(res->description()) << "</description>" << endl;
    cout << "  <license>" << xml::escape(res->licenseToConfirm()) << "</license>" << endl;

    if ( !res->repoInfo().alias().empty() )
    {
        cout << "  <source url=\"" << xml::escape(res->repoInfo().url().asString());
        cout << "\" alias=\"" << xml::escape(res->repoInfo().alias()) << "\"/>" << endl;
    }

    cout << " </update>" << endl;
  }
}

// ----------------------------------------------------------------------------

static bool list_patch_updates( Zypper & zypper )
{
  Table tbl;
  if (!Zypper::instance()->globalOpts().no_abbrev)
    tbl.allowAbbrev(5);

  Table pm_tbl; // only those that affect packagemanager (restartSuggested()), they have priority
  if (!Zypper::instance()->globalOpts().no_abbrev)
    pm_tbl.allowAbbrev(5);

  TableHeader th;
  th << _("Repository")
     << _("Name") << _("Category") << _("Severity") << _("Status") << _("Summary");
  tbl << th;
  pm_tbl << th;
  unsigned cols = th.cols();

  CliMatchPatch cliMatchPatch( zypper );
  bool all = zypper.cOpts().count("all");

  const zypp::ResPool& pool = God->pool();
  for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
  {
    Patch::constPtr patch = asKind<Patch>(*it);

    // show only needed and wanted/unlocked (bnc #420606) patches unless --all
    if ( all || (it->isBroken() && !it->isUnwanted()) )
    {
      if ( ! cliMatchPatch( patch ) )
      {
	DBG << patch->ident() << " skipped. (not matching CLI filter)" << endl;
	continue;
      }
      // table
      {
        TableRow tr (cols);
        tr << patch->repoInfo().asUserString();
        tr << patch->name ();
        tr << patch->category();
        tr << patch->severity();
        tr << (it->isBroken() ? _("needed") : _("not needed"));
        tr << patch->summary();

        if (!all && patch->restartSuggested ())
          pm_tbl << tr;
        else
          tbl << tr;
      }
    }
  }

  // those that affect the package manager go first
  // (TODO: user option for this?)
  bool affectpm = false;
  if (!pm_tbl.empty())
  {
    affectpm = true;
    if (!tbl.empty())
    {
      zypper.out().info(_("The following software management updates will be installed first:"));
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
    }
    pm_tbl.sort(1); // Name
    cout << pm_tbl;
  }

  tbl.sort(1); // Name
  if (tbl.empty() && !affectpm)
    zypper.out().info(_("No updates found."));
  else if (!tbl.empty())
  {
    if (affectpm)
    {
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
      zypper.out().info(_("The following updates are also available:"));
    }
    zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL);
    cout << tbl;
  }

  return affectpm;
}

// ----------------------------------------------------------------------------

/**
 * Find all available updates of given kind.
 */
static void
find_updates( const ResKind & kind, Candidates & candidates )
{
  const zypp::ResPool& pool = God->pool();
  DBG << "Looking for update candidates of kind " << kind << endl;

  // package updates
  if (kind == ResKind::package && !Zypper::instance()->cOpts().count("all"))
  {
    God->resolver()->doUpdate();
    ResPool::const_iterator
      it = God->pool().begin(),
      e  = God->pool().end();
    for (; it != e; ++it)
    {
      if ( (*it)->kind() != ResKind::package )
	continue;	// package updates only (bnc#779740)

      // show every package picked by doUpdate for installation
      // except the ones which are not currently installed (bnc #483910)
      if (it->status().isToBeInstalled())
      {
        ui::Selectable::constPtr s =
            ui::Selectable::get((*it)->kind(), (*it)->name());
        if (s->hasInstalledObj())
          candidates.insert(*it);
      }
    }
    return;
  }


  // get --all available updates, no matter if they are installable or break
  // some current policy
  for_(it, pool.proxy().byKindBegin(kind), pool.proxy().byKindEnd(kind))
  {
    if (!(*it)->hasInstalledObj())
      continue;

    PoolItem candidate = (*it)->highestAvailableVersionObj(); // bnc #557557
    if (!candidate)
      continue;
    if (compareByNVRA((*it)->installedObj().resolvable(), candidate.resolvable()) >= 0)
      continue;

    DBG << "selectable: " << **it << endl;
    DBG << "candidate: " << candidate << endl;
    candidates.insert (candidate);
  }
}

// ----------------------------------------------------------------------------

/**
 * Find all available updates of given kinds.
 */
void
find_updates( const ResKindSet & kinds, Candidates & candidates )
{
  for (ResKindSet::const_iterator kit = kinds.begin(); kit != kinds.end(); ++kit)
    find_updates(*kit, candidates);

  if (kinds.empty())
    WAR << "called with empty kinds set" << endl;
}

// ----------------------------------------------------------------------------

string i18n_kind_updates(const ResKind & kind)
{
  if (kind == ResKind::package)
    return _("Package updates");
  else if (kind == ResKind::patch)
    return _("Patches");
  else if (kind == ResKind::pattern)
    return _("Pattern updates");
  else if (kind == ResKind::product)
    return _("Product updates");

  return boost::str(format("%s updates") % kind);
}

// ----------------------------------------------------------------------------

// FIXME rewrite this function so that first the list of updates is collected and later correctly presented (bnc #523573)

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort)
{
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<update-status version=\"0.6\">" << endl;
    cout << "<update-list>" << endl;
  }

  // whether some of the listed patches affects package management itself
  // false indicates that we are not checking for patches at all
  // (no 'zypper lp' or 'zypper lu -t patch ...'), or there are no patches
  // affecting the package management stack
  bool affects_pkgmgr = false;

  unsigned kind_size = kinds.size();
  ResKindSet localkinds = kinds;
  ResKindSet::iterator it;

  // patch updates first
  it = localkinds.find(ResKind::patch);
  if(it != localkinds.end())
  {
    if (zypper.out().type() == Out::TYPE_XML)
      affects_pkgmgr = xml_list_patches(zypper);
    else
    {
      if (kinds.size() > 1)
      {
        zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
        zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      }
      affects_pkgmgr = list_patch_updates(zypper);
    }
    localkinds.erase(it);
  }

  // list other kinds (only if there are no _patches_ affecting the package manager)

  // XML output here
  if (zypper.out().type() == Out::TYPE_XML)
  {
    if (!affects_pkgmgr)
      xml_list_updates(localkinds);
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;
    return;
  }

  if (affects_pkgmgr)
    return;

  // normal output here
  for (it = localkinds.begin(); it != localkinds.end(); ++it)
  {
    Table tbl;

    // show repo only if not best effort or --from-repo set
    // on best_effort, the solver will determine the repo if we don't limit it to a specific one
    bool hide_repo = best_effort || copts.count("repo");

    // header
    TableHeader th;
    unsigned int name_col;
    // TranslatorExplanation S stands for Status
    th << _("S");
    if (!hide_repo)
      th << _("Repository");

    if (zypper.globalOpts().is_rug_compatible)
      th << _("Bundle");

    name_col = th.cols();
    th << _("Name");
    // best_effort does not know version or arch yet
    if (!best_effort)
    {
      if (*it == ResKind::package)
        th << table::EditionStyleSetter( tbl, _("Current Version") );
      th << table::EditionStyleSetter( tbl, _("Available Version") ) << _("Arch");
    }

    tbl << th;

    unsigned int cols = th.cols();

    ResPoolProxy uipool( ResPool::instance().proxy() );

    Candidates candidates;
    find_updates( *it, candidates );

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
        tr << res->repoInfo().asUserString();
      }
      if (zypper.globalOpts().is_rug_compatible)
        tr << "";               // Bundle
      tr << res->name ();

      // strictly speaking, we could show version and arch even in best_effort
      //  iff there is only one candidate. But we don't know the number of candidates here.
      if (!best_effort)
      {
        // for packages show also the current installed version (bnc #466599)
        if (*it == ResKind::package)
        {
          ui::Selectable::Ptr sel( uipool.lookup( *ci ) );
          if ( sel->hasInstalledObj() )
            tr << sel->installedObj()->edition().asString();
        }
        tr << res->edition ().asString ()
          << res->arch ().asString ();
      }
      tbl << tr;
    }
    tbl.sort( name_col );

    if (kind_size > 1)
    {
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
      zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
    }

    if (tbl.empty())
      zypper.out().info(_("No updates found."));
    else
      cout << tbl;
  }
}

// ----------------------------------------------------------------------------

void list_patches_by_issue( Zypper & zypper )
{
  // --bz, --cve can't be used together with --issue; this case is ruled out
  // in the initial arguments validation in Zypper.cc
  Table t;
  t << ( TableHeader() << _("Issue") << _("No.") << _("Patch") << _("Category") << _("Severity") << _("Status") );

  CliScanIssues issues;
  CliMatchPatch cliMatchPatch( zypper );
  bool only_needed = !zypper.cOpts().count("all");

  // Basic PoolQuery tuned for each argument
  PoolQuery basicQ;
  basicQ.setMatchSubstring();
  basicQ.setCaseSensitive( false );
  basicQ.addKind( ResKind::patch );

  std::vector<const Issue*> pass2; // on the fly remember anyType issues for pass2

  for ( const Issue & issue : issues )
  {
    DBG << "querying: " << issue.type() << " = " << issue.id() << endl;
    PoolQuery q( basicQ );
    // PoolQuery ORs attributes but we need AND.
    // Post processing the match must assert correct type of specific IDs!
    if ( issue.specificType() && issue.anyId() )
    { q.addAttribute(sat::SolvAttr::updateReferenceType, issue.type() ); }
    else
    {
      q.addAttribute( sat::SolvAttr::updateReferenceId, issue.id() );
      if ( issue.anyType() && issue.specificId() ) // remember for pass2
      { pass2.push_back( &issue ); }
    }

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi( *it );
      Patch::constPtr patch = asKind<Patch>(pi);

      if ( only_needed && ( !pi.isBroken() || pi.isUnwanted() ) )
	continue;

      if ( ! cliMatchPatch( patch ) )
      {
	DBG << patch->ident() << " skipped. (not matching CLI filter)" << endl;
	continue;
      }

      // Print details about each match in that solvable:
      for_( d, it.matchesBegin(), it.matchesEnd() )
      {
        const std::string & itype = d->subFind( sat::SolvAttr::updateReferenceType ).asString();

	if ( issue.specificType() && itype != issue.type() )
	  continue;	// assert correct type of specific IDs

	t << ( TableRow()
	  << itype
	  << d->subFind( sat::SolvAttr::updateReferenceId ).asString()
	  << patch->name()
	  << patch->category()
	  << patch->severity()
	  << (pi.isBroken() ? _("needed") : _("not needed")) );
      }
    }
  }

  // pass2: look for matches in patch summary/description
  //
  Table t1;
  t1 << ( TableHeader() << _("Name") << _("Category") << _("Severity") << _("Summary") );

  for ( const Issue* _issue : pass2 )
  {
    const Issue & issue( *_issue );
    PoolQuery q( basicQ );
    q.addAttribute(sat::SolvAttr::summary, issue.id() );
    q.addAttribute(sat::SolvAttr::description, issue.id() );

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi( *it );
      Patch::constPtr patch = asKind<Patch>(pi);

      if ( only_needed && ( !pi.isBroken() || pi.isUnwanted() ) )
	continue;

      if ( ! cliMatchPatch( patch ) )
      {
	DBG << patch->ident() << " skipped. (not matching CLI filter)" << endl;
	continue;
      }

      t1 << ( TableRow()
         << patch->name() << patch->category() << patch->severity() << patch->summary() );
      //! \todo could show a highlighted match with a portion of surrounding
      //! text. Needs case-insensitive find.
    }
  }

  // print result
  if ( !zypper.globalOpts().no_abbrev )
    t1.allowAbbrev(3);
  t.sort(3);
  t1.sort(0);

  if ( t.empty() && t1.empty() )
  {  zypper.out().info(_("No matching issues found.")); }
  else
  {
    if ( !t.empty() )
    {
      if ( !pass2.empty() )
      {
        cout << endl;
        zypper.out().info(_("The following matches in issue numbers have been found:"));
      }
      cout << endl << t;
    }

    if ( !t1.empty() )
    {
      if ( !t.empty() )
      { cout << endl; }
      zypper.out().info(_( "Matches in patch descriptions of the following patches have been found:"));
      cout << endl << t1;
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates_by_issue( Zypper & zypper )
{
  CliScanIssues issues;

  // Basic PoolQuery tuned for each argument
  PoolQuery basicQ;
  basicQ.setMatchExact();
  basicQ.setCaseSensitive( false );
  basicQ.addKind( ResKind::patch );

  SolverRequester::Options srOpts;
  srOpts.force = zypper.cOpts().count("force");
  srOpts.skip_interactive = zypper.cOpts().count("skip-interactive");
  srOpts.cliMatchPatch = CliMatchPatch( zypper );

  for ( const Issue & issue : issues )
  {
    PoolQuery q( basicQ );
    // PoolQuery ORs attributes but we need AND.
    // Post processing the match must assert correct type of specific IDs!
    if ( issue.specificType() && issue.anyId() )
    { q.addAttribute(sat::SolvAttr::updateReferenceType, issue.type() ); }
    else
    { q.addAttribute( sat::SolvAttr::updateReferenceId, issue.id() ); }

    SolverRequester sr( srOpts );
    bool found = false;

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi( *it );
      Patch::constPtr patch = asKind<Patch>(pi);

      if ( !pi.isBroken() ) // not needed
	continue;

      // CliMatchPatch not needed, it's fed into srOpts!

      DBG << "got: " << *it << endl;

      for_( d, it.matchesBegin(), it.matchesEnd() )
      {
	const std::string & itype = d->subFind( sat::SolvAttr::updateReferenceType ).asString();

	if ( issue.specificType() && itype != issue.type() )
	  continue;	// assert correct type of specific IDs

	if ( sr.installPatch( pi ) )
	  found = true;
	else
	  DBG << str::form("fix for %s issue number %s was not marked.",
			   issue.type().c_str(), issue.id().c_str() );
      }
    }

    sr.printFeedback( zypper.out() );
    if ( ! found )
    {
      const std::string & itype( issue.type() );
      if ( itype == "bugzilla" )
	zypper.out().info(str::form(_("Fix for bugzilla issue number %s was not found or is not needed."), issue.id().c_str() ));
      else if ( itype == "cve" )
	zypper.out().info(str::form(_("Fix for CVE issue number %s was not found or is not needed."), issue.id().c_str() ));
      else
	// translators: keep '%s issue' together, it's something like 'CVE issue' or 'Bugzilla issue'
	zypper.out().info(str::form(_("Fix for %s issue number %s was not found or is not needed."), itype.c_str(), issue.id().c_str() ));
      zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    }
  } // next issue from --bz --cve
}
