#include <iostream> // for xml and table output
#include <sstream>

#include <zypp/base/LogTools.h>
#include <zypp/ZYppFactory.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Iterable.h>
#include <zypp/PoolQuery.h>

#include <zypp/Patch.h>

#include "SolverRequester.h"
#include "Table.h"
#include "update.h"
#include "main.h"

using namespace zypp;
typedef std::set<PoolItem> Candidates;

extern ZYpp::Ptr God;

static void find_updates( const ResKindSet & kinds, Candidates & candidates );

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
    //        cliOption   issueType
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

///////////////////////////////////////////////////////////////////
namespace
{
  inline bool patchIsApplicable( const PoolItem & pi )	///< Default content for all patch lists: applicable (needed, optional, unwanted)
  { return pi.isBroken(); }

  inline bool patchIsNeededRestartSuggested( const PoolItem & pi )	///< Needed update stack pack; installed first!
  {
    return pi.isBroken()
    && ! pi.isUnwanted()
    && ! ( Zypper::instance()->globalOpts().exclude_optional_patches && pi->asKind<Patch>()->categoryEnum() == Patch::CAT_OPTIONAL )
    && pi->asKind<Patch>()->restartSuggested();
  }

} //namespace
///////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////
namespace
{
  /// Count patches per category and class ()
  ///
  ///  Category    | Updatestack | Patches | locked
  ///  --------------------------------------------
  ///  security    | N S         | N S     | L
  ///  recommended | N           | N       | L
  ///  other       | N           | N       | L
  ///  optional    | O(N)        | O(N)    | L
  ///
  ///  N = needed
  ///  S = needed security
  ///  O = optional (if excludeOptionalPatches, otherwise N)
  ///  L = locked
  ///
  ///  N + O + L == collected
  ///
  ///
  ///
  struct PatchCheckStats
  {

    explicit PatchCheckStats( bool excludeOptionalPatches_r )
    : _excludeOptionalPatches( excludeOptionalPatches_r )
    {}

    /** Optionally track total amount of applicable patches */
    bool visit( const PoolItem & pi_r )
    { bool ret = pi_r.isBroken(); if ( ret ) ++_visited; return ret; }

    /** Optionally track total amount of applicable patches */
    void visit()
    { ++_visited; }

    /** Contributing to the stats */
    void collect( const PoolItem & pi_r )
    {
      Patch::constPtr patch( pi_r->asKind<Patch>() );
      if ( patch )
      { collect( pi_r, patch ); }
    }

    unsigned visited() const	{ return _visited; }
    unsigned collected() const	{ return _collected; }

    unsigned needed() const	{ return _needed; }
    unsigned security() const	{ return _security; }
    unsigned optional() const	{ return _optional; }
    unsigned locked() const	{ return _locked; }

  public:
    void render( Out & out, bool withDetails_r ) const;
    void renderDetails( Out & out ) const;

  private:
    typedef ZeroInit<unsigned> Counter;

    enum Level
    {
      kUSTACK,
      kNEEDED,
      kLOCKED,
      kTOTAL,	// LAST ENTRY!
    };

    struct Stats : public std::vector<Counter>
    {
      Stats() : std::vector<Counter>( kTOTAL ) {}
      std::set<std::string> _aka;
    };

    struct CategorySort
    {
      bool operator()( const Patch::Category & lhs, const Patch::Category & rhs )
      {
	if ( lhs == rhs )
	  return false;

	// top
	if ( lhs == Patch::CAT_SECURITY )
	  return true;
	if ( rhs == Patch::CAT_SECURITY )
	  return false;

	if ( lhs == Patch::CAT_RECOMMENDED )
	  return true;
	if ( rhs == Patch::CAT_RECOMMENDED )
	  return false;

	// bottom
	if ( lhs == Patch::CAT_OPTIONAL )
	  return false;
	if ( rhs == Patch::CAT_OPTIONAL )
	  return true;

	// the remaining ones in between
	return lhs > rhs;
      }
    };

    typedef std::map<Patch::Category, Stats, CategorySort> StatsMap;

  private:
    void collect( const PoolItem & pi_r, const Patch::constPtr & patch_r )
    {
      ++_collected;
      Level level = pi_r.isUnwanted() ? PatchCheckStats::kLOCKED
				      : ( patch_r->restartSuggested() ? PatchCheckStats::kUSTACK
								      : PatchCheckStats::kNEEDED );

      Patch::Category cat = patch_r->categoryEnum();
      if ( level == kLOCKED )
	++_locked;
      else if ( _excludeOptionalPatches && cat == Patch::CAT_OPTIONAL )
	++_optional;
      else
      {
	++_needed;
	if ( cat == Patch::CAT_SECURITY )
	  ++_security;
      }

      // detailed stats:
      Stats & detail( _stats[cat] );
      ++detail[level];
      const std::string & ctgry( patch_r->category() );	// on the fly remember aliases, e.g. 'feature' == 'optional'
      if ( asString( cat ) != ctgry )
	detail._aka.insert( ctgry );
    }

    std::string renderCounter( const Counter & counter_r ) const
    { return counter_r ? asString(counter_r) : "-"; }

  private:
    bool	_excludeOptionalPatches;	/// if true, optional patches are counted separately
    Counter	_visited;	///< if >_collected: "only _collected out of _visited have been considered"
    Counter	_collected;	///< count contributed to the stats
    Counter	_needed;
    Counter	_security;
    Counter	_optional;
    Counter	_locked;
    StatsMap	_stats;		///< counter table
  };

  void PatchCheckStats::render( Out & out, bool withDetails_r ) const
  {
    if ( visited() )
    {
      if ( visited() && visited() > collected() )
	// translator: stats table header (plural number is %2%)
	out.info( str::Format(PL_("Considering %1% out of %2% applicable patches:",
				  "Considering %1% out of %2% applicable patches:", visited())) % collected() % visited() );
	else
	  // translator: stats table header
	  out.info( str::Format(PL_("Found %1% applicable patch:", "Found %1% applicable patches:", collected())) % collected() );

	if ( collected() )
	{
	  if ( withDetails_r )
	    renderDetails( out );

	  if ( locked() )
	  {
	    // translator: stats summary
	    out.info( ColorString( str::Format(PL_("%d patch locked", "%d patches locked", locked())) % locked(),
				   ColorContext::HIGHLIGHT ).str(),
		      Out::QUIET );
	  }

	  if ( optional() )	// only if exclude_optional_patches
	  {
	    // translator: stats summary
	    out.infoLRHint( ColorString( str::Format(PL_("%d patch optional", "%d patches optional", optional())) % optional(),
					 ColorContext::LOWLIGHT ).str(),
			    // translator: Hint displayed right adjusted; %1% is a CLI option
			    // "42 patches optional                  (use --with-optional to include optional patches)"
			    str::Format(_("use '%1%' to include optional patches")) % "--with-optional",
			    Out::QUIET );
	  }
	}
    }
    // always:
    {
      std::ostringstream s;
      // translator: stats summary
      s << str::Format(PL_("%d patch needed", "%d patches needed", needed())) % needed()
      << " ("
      // translator: stats summary
      <<  str::Format(PL_("%d security patch", "%d security patches", security())) % security()
      << ")";
      out.info( s.str(), Out::QUIET );
    }
  }

  void PatchCheckStats::renderDetails( Out & out ) const
  {
    if ( out.typeNORMAL() )
    {
      ColorContext ctxtOptional = ( _excludeOptionalPatches ? ColorContext::LOWLIGHT : ColorContext::DEFAULT );
      ColorContext ctxtLocked   = ColorContext::HIGHLIGHT;

      bool haveUPD = false;
      bool havePAT = false;
      bool haveLCK = false;
      bool haveAKA = false;
      // 1st pass
      for ( const auto & p : _stats )
      {
	const Stats & stats( p.second );
	if ( !haveUPD && stats[kUSTACK] )	haveUPD = true;
	if ( !havePAT && stats[kNEEDED])	havePAT = true;
	if ( !haveLCK && stats[kLOCKED])	haveLCK = true;
	if ( !haveAKA && !stats._aka.empty() )	haveAKA = true;
      }

      // 2nd pass
      Table tbl;
      {
	TableHeader hdr;
	// translator: Table column header.
	hdr << _("Category");
	// translator: Table column header.
	if ( haveUPD )	hdr << _("Updatestack");
	// translator: Table column header.
	if ( havePAT )	hdr << _("Patches");
	// translator: Table column header.
	if ( haveLCK )	hdr << ColorString( ctxtLocked, _("Locked") );
	// translator: Table column header
	// Used if stats collect data for more than one category name.
	// Category    | Updatestack | Patches | Locked | Included categories
	// ------------+-------------+---------+--------+---------------------
	//  optional   | ...                      ..... | enhancement, feature
	if ( haveAKA )	hdr << _("Included categories");
	tbl << std::move(hdr);
      }

      for ( const auto & p : _stats )
      {
	const Patch::Category & category( p.first );
	const Stats & stats( p.second );

	TableRow row( category == Patch::CAT_OPTIONAL ? ctxtOptional : ColorContext::DEFAULT );
	row << category;
	if ( haveUPD )	row << renderCounter(stats[kUSTACK]);
	if ( havePAT )	row << renderCounter(stats[kNEEDED]);
	if ( haveLCK )	row << ColorString( ctxtLocked, renderCounter(stats[kLOCKED]) );
	if ( haveAKA )	row << ( stats._aka.empty() ? "" : str::join( stats._aka, ", " ) );
	tbl << std::move(row);
      }
      cout << tbl;
      out.gap();
    }
  }
} // namespace
///////////////////////////////////////////////////////////////////

void patch_check()
{
  Zypper & zypper( *Zypper::instance() );
  Out & out( zypper.out() );
  DBG << "patch check" << endl;

  PatchCheckStats stats( zypper.globalOpts().exclude_optional_patches );
  bool updatestackOnly = Zypper::instance()->cOpts().count("updatestack-only");
  for_( it, God->pool().byKindBegin(ResKind::patch), God->pool().byKindEnd(ResKind::patch) )
  {
    const PoolItem & pi( *it );
    if ( ! stats.visit( pi ) )	// count total applicable patches
      continue;

    // filter out by cli options
    if ( updatestackOnly && !pi->asKind<Patch>()->restartSuggested() )
      continue;

    // remaining: collect stats
    stats.collect( pi );
  }

  // render output
  out.gap();
  stats.render( out, /*withDetails*/true );

  // compute exit code
  if ( stats.needed() )
  { zypper.setExitCode( stats.security() ? ZYPPER_EXIT_INF_SEC_UPDATE_NEEDED : ZYPPER_EXIT_INF_UPDATE_NEEDED ); }
}

static void xml_print_patch( Zypper & zypper, const PoolItem & pi )
{
  Patch::constPtr patch = pi->asKind<Patch>();

  cout << " <update ";
  cout << "name=\"" << patch->name () << "\" ";
  cout << "edition=\""  << patch->edition() << "\" ";
  cout << "arch=\""  << patch->arch() << "\" ";
  cout << "status=\""  << textPatchStatus( pi ) << "\" ";
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


// returns true if NEEDED! restartSuggested() patches are available
static bool xml_list_patches (Zypper & zypper)
{
  const ResPool& pool = God->pool();

  // check whether there are packages affecting the update stack
  bool pkg_mgr_available = false;
  for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
  {
    if ( patchIsNeededRestartSuggested( *it ) )
    {
      pkg_mgr_available = true;
      break;
    }
  }

  unsigned patchcount = 0;
  bool all = zypper.cOpts().count("all");
  for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
  {
    if ( all || patchIsApplicable( *it ) )
    {
      const PoolItem & pi( *it );
      Patch::constPtr patch = pi->asKind<Patch>();

      // if updates stack patches are available, show only those
      if ( all || !pkg_mgr_available || patchIsNeededRestartSuggested( pi ) )
      {
	xml_print_patch( zypper, pi );
      }
    }
    ++patchcount;
  }

  //! \todo change this from appletinfo to something general, define in xmlout.rnc
  if (patchcount == 0)
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;


  if ( pkg_mgr_available )
  {
    // close <update-list> and write <blocked-update-list> if not all
    cout << "</update-list>" << endl;
    if ( ! all )
    {
    cout << "<blocked-update-list>" << endl;
    for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
    {
      if ( patchIsApplicable( *it ) )
      {
	const PoolItem & pi( *it );
	Patch::constPtr patch = pi->asKind<Patch>();
	if ( ! patchIsNeededRestartSuggested( pi ) )
	  xml_print_patch( zypper, pi );
      }
    }
    cout << "</blocked-update-list>" << endl;
    }
  }

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void xml_list_updates(const ResKindSet & kinds)
{
  Candidates candidates;
  find_updates( kinds, candidates );

  for( const PoolItem & pi : candidates )
  {
    cout << " <update ";
    cout << "name=\"" << pi.name () << "\" " ;
    cout << "edition=\""  << pi.edition() << "\" ";
    cout << "arch=\""  << pi.arch() << "\" ";
    cout << "kind=\"" << pi.kind() << "\" ";
    // for packages show also the current installed version (bnc #466599)
    {
      const PoolItem & ipi( ui::Selectable::get(pi)->installedObj() );
      if ( ipi )
      {
	if ( pi.edition() != ipi.edition() )
	  cout << "edition-old=\""  << ipi.edition() << "\" ";
	if ( pi.arch() != ipi.arch() )
	  cout << "arch-old=\""  << ipi.arch() << "\" ";
      }
    }
    cout << ">" << endl;
    cout << "  <summary>" << xml::escape(pi.summary()) << "</summary>" << endl;
    cout << "  <description>" << xml::escape(pi.description()) << "</description>" << endl;
    cout << "  <license>" << xml::escape(pi.licenseToConfirm()) << "</license>" << endl;

    if ( !pi.repoInfo().alias().empty() )
    {
        cout << "  <source url=\"" << xml::escape(pi.repoInfo().url().asString());
        cout << "\" alias=\"" << xml::escape(pi.repoInfo().alias()) << "\"/>" << endl;
    }

    cout << " </update>" << endl;
  }
}

// ----------------------------------------------------------------------------

// returns true if NEEDED! restartSuggested() patches are available
static bool list_patch_updates( Zypper & zypper )
{
  Table tbl;
  FillPatchesTable intoTbl( tbl );

  Table pmTbl; // only NEEDED! that affect packagemanager (restartSuggested()), they have priority
  FillPatchesTable intoPMTbl( pmTbl );

  PatchCheckStats stats( zypper.globalOpts().exclude_optional_patches );
  CliMatchPatch cliMatchPatch( zypper );
  bool all = zypper.cOpts().count("all");

  const ResPool& pool = God->pool();
  for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
  {
    const PoolItem & pi( *it );
    Patch::constPtr patch = asKind<Patch>(pi);

    bool tostat = stats.visit( pi );	// count total applicable patches

    if ( ! cliMatchPatch( patch ) )
      continue;

    if ( tostat )	// exclude cliMatchPatch filtered but include undisplayed ones
      stats.collect( pi );

    if ( all || patchIsApplicable( pi ) )
    {
      if ( ! all && patchIsNeededRestartSuggested( pi ) )
	intoPMTbl( pi );
      else
	intoTbl( pi );
    }
  }

  // those that affect the package manager go first
  bool affectpm = !pmTbl.empty();
  if ( affectpm )
  {
    zypper.out().gap();
    if (!tbl.empty())
    {
#if 0
      N_("The following software management updates will be installed first:"); // keep old text for a while...
#endif
      // translator: Table headline; 'Needed' refers to patches with status 'needed'
      zypper.out().info(_("Needed software management updates will be installed first:"));
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
    }
    pmTbl.sort();	// use default sort
    cout << pmTbl;
  }

  if (tbl.empty() && !affectpm)
    zypper.out().info(_("No updates found."));
  else if (!tbl.empty())
  {
    if (affectpm)
    {
      zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
      // translator: Table headline
      zypper.out().info(_("The following updates are also available:"));
    }
    zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL);
    tbl.sort();	// use default sort
    cout << tbl;
  }

  zypper.out().gap();
  if ( stats.visited() )
  {
    stats.render( zypper.out(), /*withDetails*/false );
    zypper.out().gap();
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
  const ResPool& pool = God->pool();
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
    if (compareByNVRA((*it)->installedObj(), candidate) >= 0)
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

std::string i18n_kind_updates(const ResKind & kind)
{
  if (kind == ResKind::package)
    return _("Package updates");
  else if (kind == ResKind::patch)
    return _("Patches");
  else if (kind == ResKind::pattern)
    return _("Pattern updates");
  else if (kind == ResKind::product)
    return _("Product updates");

  return str::Format("%s updates") % kind;
}

// ----------------------------------------------------------------------------

// FIXME rewrite this function so that first the list of updates is collected and later correctly presented (bnc #523573)

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort)
{
  if (zypper.out().type() == Out::TYPE_XML)
  {
    // TODO: go for XmlNode
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
    {
      xml_list_updates(localkinds);
      cout << "</update-list>" << endl;		// otherwise closed in xml_list_patches
    }
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
    unsigned name_col;
    // TranslatorExplanation S stands for Status
    th << _("S");
    if (!hide_repo)
      th << _("Repository");

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

    unsigned cols = th.cols();

    ResPoolProxy uipool( ResPool::instance().proxy() );

    Candidates candidates;
    find_updates( *it, candidates );

    for ( const PoolItem & pi : candidates )
    {
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
        tr << pi.repoInfo().asUserString();
      }
      tr << pi.name ();

      // strictly speaking, we could show version and arch even in best_effort
      //  iff there is only one candidate. But we don't know the number of candidates here.
      if (!best_effort)
      {
        // for packages show also the current installed version (bnc #466599)
        if (*it == ResKind::package)
        {
          ui::Selectable::Ptr sel( uipool.lookup( pi ) );
          if ( sel->hasInstalledObj() )
            tr << sel->installedObj()->edition();
        }
        tr << pi.edition()
          << pi.arch();
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
  Table issueMatchesTbl;
  FillPatchesTableForIssue intoIssueMatchesTbl( issueMatchesTbl );

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
    PoolQuery q( basicQ );
    // PoolQuery ORs attributes but we need AND.
    // Post processing the match must assert correct type of specific IDs!
    if ( issue.specificType() && issue.anyId() )
    { q.addAttribute(sat::SolvAttr::updateReferenceType, issue.type() ); }
    else
    {
      q.addAttribute( sat::SolvAttr::updateReferenceId, issue.id() );
      if ( issue.anyType() && issue.specificId() ) 				// remember for pass2
      {
	q.addAttribute( sat::SolvAttr::updateReferenceType, issue.id() );	// bnc#941309: let '--issue-bugzilla' also match the type
	pass2.push_back( &issue );
      }
    }

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi( *it );
      Patch::constPtr patch = asKind<Patch>(pi);

      if ( only_needed && ! patchIsApplicable( pi ) )
	continue;

      if ( ! cliMatchPatch( patch ) )
      {
	DBG << patch->ident() << " skipped. (not matching CLI filter)" << endl;
	continue;
      }

      // Print details about each match in that solvable:
      // NOTE: If searching in BOTH updateReferenceId AND updateReferenceType,
      // we may find matches in both but want to report the issue just once!
      std::set<std::pair<std::string,std::string>> unifiedResults;
      for_( d, it.matchesBegin(), it.matchesEnd() )
      {
        std::string itype = d->subFind( sat::SolvAttr::updateReferenceType ).asString();

	if ( issue.specificType() && itype != issue.type() )
	  continue;	// assert correct type of specific IDs

	// remember....
	unifiedResults.insert( std::make_pair( std::move(itype), d->subFind( sat::SolvAttr::updateReferenceId ).asString() ) );
      }
      // print remembered...
      for ( auto & p : unifiedResults )
      {
	intoIssueMatchesTbl( pi, std::move(p.first), std::move(p.second) );
      }
    }
  }

  // pass2: look for matches in patch summary/description
  //
  Table summaryMatchesTbl;
  FillPatchesTable intoSummaryMatchesTbl( summaryMatchesTbl );

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

      if ( only_needed && ! patchIsApplicable( pi ) )
	continue;

      if ( ! cliMatchPatch( patch ) )
      {
	DBG << patch->ident() << " skipped. (not matching CLI filter)" << endl;
	continue;
      }

      intoSummaryMatchesTbl( pi );
//	issueMatchesTbl.rows().back().addDetail( str::Str() << "    " << attr.inSolvAttr() << "\t\"" << attr.asString() << "\"" );
      //! \todo could show a highlighted match with a portion of surrounding
      //! text. Needs case-insensitive find.
    }
  }

  // print result
  if ( issueMatchesTbl.empty() && summaryMatchesTbl.empty() )
  {  zypper.out().info(_("No matching issues found.")); }
  else
  {
    if ( !issueMatchesTbl.empty() )
    {
      if ( !pass2.empty() )
      {
        cout << endl;
        zypper.out().info(_("The following matches in issue numbers have been found:"));
      }
      issueMatchesTbl.sort(); // use default sort
      cout << endl << issueMatchesTbl;
    }

    if ( !summaryMatchesTbl.empty() )
    {
      if ( !issueMatchesTbl.empty() )
      { cout << endl; }
      zypper.out().info(_( "Matches in patch descriptions of the following patches have been found:"));
      summaryMatchesTbl.sort(); // use default sort
      cout << endl << summaryMatchesTbl;
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
  srOpts.skip_optional_patches = zypper.globalOpts().exclude_optional_patches;
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
