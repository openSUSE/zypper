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
#include "global-settings.h"
#include "utils/misc.h"

using namespace zypp;
typedef std::set<PoolItem> Candidates;

extern ZYpp::Ptr God;

static void find_updates( const ResKindSet & kinds, Candidates & candidates, bool all_r );

///////////////////////////////////////////////////////////////////
/// will go into next libzypp
namespace zypp_pending
{
  template<class TMap>
  Iterable<typename MapKVIteratorTraits<TMap>::Key_const_iterator> make_map_key_Iterable( const TMap & map_r )
  {
    return makeIterable( make_map_key_begin( map_r ), make_map_key_end( map_r ) );
  }
} // namespce zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace
{
  inline bool patchIsApplicable( const PoolItem & pi )	///< Default content for all patch lists: applicable (needed, optional, unwanted)
  { return pi.isBroken(); }

  inline bool patchIsNeededRestartSuggested( const PoolItem & pi )	///< Needed update stack pack; installed first!
  {
    return pi.isBroken()
    && ! pi.isUnwanted()
    && ! ( Zypper::instance().config().exclude_optional_patches && pi->asKind<Patch>()->categoryEnum() == Patch::CAT_OPTIONAL )
    && pi->asKind<Patch>()->restartSuggested();
  }

  /** RNC: Print other-update element */
  inline std::ostream & xmlPrintOtherUpdateOn( std::ostream & str, const PoolItem & pi_r )
  {
    xmlout::Node parent { cout, "update", xmlout::Node::optionalContent, {
      { "kind", pi_r.kind() },
      { "name", pi_r.name () },
      { "edition", pi_r.edition() },
      { "arch", pi_r.arch() },
    } };
    // for packages show also the current installed version (bnc #466599)
    {
      const PoolItem & ipi( ui::Selectable::get(pi_r)->installedObj() );
      if ( ipi )
      {
        if ( pi_r.edition() != ipi.edition() )
          parent.addAttr( { "edition-old", ipi.edition() } );
        if ( pi_r.arch() != ipi.arch() )
          parent.addAttr( { "arch-old", ipi.arch() } );
      }
    }

    dumpAsXmlOn( *parent, pi_r.summary(), "summary" );
    dumpAsXmlOn( *parent, pi_r.description(), "description" );
    dumpAsXmlOn( *parent, pi_r.licenseToConfirm(), "license" );

    if ( !pi_r.repoInfo().alias().empty() )
    {
      xmlout::Node( *parent, "source", xmlout::Node::optionalContent, {
        { "url", pi_r.repoInfo().url().asString() },
        { "alias", pi_r.repoInfo().alias() },
      } );
    }
    return str;
  }

  /** RNC: Print patch-issue-list-element */
  inline std::ostream & xmlPrintPatchIssueListElementOn( std::ostream & str, Patch::ReferenceIterator begin_r, Patch::ReferenceIterator end_r )
  {
    xmlout::Node parent { str, "issue-list", xmlout::Node::optionalContent };
    for_( it, begin_r, end_r )
    {
      xmlout::Node issue { *parent, "issue", xmlout::Node::optionalContent, {
        { "type", it.type() },
        { "id", it.id() },
      } };
      zypp::dumpAsXmlOn( *issue, it.title(), "title" );
      zypp::dumpAsXmlOn( *issue, it.href(), "href" );
    }
    return str;
  }

  /** RNC: Print patch-update element */
  inline std::ostream & xmlPrintPatchUpdateOn( std::ostream & str, const PoolItem & pi_r, const PatchHistoryData & patchHistoryData_r )
  {
    Patch::constPtr patch = pi_r->asKind<Patch>();

    Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
    if ( Zypper::instance().config().reboot_req_non_interactive )
      ignoreFlags |= Patch::Reboot;
    if ( LicenseAgreementPolicy::instance()._autoAgreeWithLicenses )
      ignoreFlags |= Patch::License;

    // write the node
    xmlout::Node parent { str, "update", xmlout::Node::optionalContent, {
      { "kind", "patch" },
      { "name", patch->name () },
      { "edition", patch->edition() },
      { "arch", patch->arch() },
      { "status", textPatchStatus( pi_r ) },
      { "category", patch->category() },
      { "severity", patch->severity() },
      { "pkgmanager", asString( patch->restartSuggested() ) },
      { "restart", asString( patch->rebootSuggested() ) },
      { "interactive", asString( patch->interactiveWhenIgnoring(ignoreFlags) ) },
    } };

    if ( PatchHistoryData::value_type res { patchHistoryData_r[pi_r] }; res != PatchHistoryData::noData )
    {
      if ( res.second == pi_r.status().validate() )
        dumpAsXmlOn( *parent, res.first, "status-since" );
      else
        // patch status was changed by a non zypp transaction (not mentioned in the history)
        DBG << "PatchHistoryData " << res.second << " but " << pi_r << endl;
    }
    dumpAsXmlOn( *parent, patch->summary(), "summary" );
    dumpAsXmlOn( *parent, patch->description(), "description" );
    dumpAsXmlOn( *parent, patch->licenseToConfirm(), "license" );

    if ( !patch->repoInfo().alias().empty() )
    {
      xmlout::Node( *parent, "source", xmlout::Node::optionalContent, {
        { "url", patch->repoInfo().url().asString() },
        { "alias", patch->repoInfo().alias() },
      } );
    }

    dumpAsXmlOn( *parent, patch->timestamp(), "issue-date" );
    xmlPrintPatchIssueListElementOn( *parent, patch->referencesBegin(), patch->referencesEnd() );
    return str;
  }

  /** RNC: Print element containing patch-update-list */
  template <typename TContainer>
  inline std::ostream & xmlPrintPatchUpdateListOn( std::ostream & str, std::string nodename_r, TContainer container_r, const PatchHistoryData & patchHistoryData_r )
  {
    xmlout::Node parent { str, std::move(nodename_r), xmlout::Node::optionalContent };
    for ( const auto & pi : container_r )
      xmlPrintPatchUpdateOn( *parent, pi, patchHistoryData_r );
    return str;
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
      bool operator()( const Patch::Category & lhs, const Patch::Category & rhs ) const
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
        hdr << N_("Category");
        // translator: Table column header.
        if ( haveUPD )	hdr << N_("Updatestack");
        // translator: Table column header.
        if ( havePAT )	hdr << N_("Patches");
        // translator: Table column header.
        if ( haveLCK )	hdr << ColorString( ctxtLocked, N_("Locked") );
        // translator: Table column header
        // Used if stats collect data for more than one category name.
        // Category    | Updatestack | Patches | Locked | Included categories
        // ------------+-------------+---------+--------+---------------------
        //  optional   | ...                      ..... | enhancement, feature
        if ( haveAKA )	hdr << N_("Included categories");
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

void patch_check( bool updatestackOnly )
{
  Zypper & zypper( Zypper::instance() );
  Out & out( zypper.out() );
  DBG << "patch check" << endl;

  PatchCheckStats stats( zypper.config().exclude_optional_patches );
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

// returns true if NEEDED! restartSuggested() patches are available
static bool xml_list_patches (Zypper & zypper, bool all_r, const PatchHistoryData & patchHistoryData_r )
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
  for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
  {
    if ( all_r || patchIsApplicable( *it ) )
    {
      const PoolItem & pi( *it );
      Patch::constPtr patch = pi->asKind<Patch>();

      // if updates stack patches are available, show only those
      if ( all_r || !pkg_mgr_available || patchIsNeededRestartSuggested( pi ) )
      {
        xmlPrintPatchUpdateOn( cout, pi, patchHistoryData_r );
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
    if ( ! all_r )
    {
    cout << "<blocked-update-list>" << endl;
    for_( it, pool.byKindBegin(ResKind::patch), pool.byKindEnd(ResKind::patch) )
    {
      if ( patchIsApplicable( *it ) )
      {
        const PoolItem & pi( *it );
        Patch::constPtr patch = pi->asKind<Patch>();
        if ( ! patchIsNeededRestartSuggested( pi ) )
          xmlPrintPatchUpdateOn( cout, pi, patchHistoryData_r );
      }
    }
    cout << "</blocked-update-list>" << endl;
    }
  }

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void xml_list_updates(const ResKindSet & kinds, bool all_r )
{
  Candidates candidates;
  find_updates( kinds, candidates, all_r );

  for( const PoolItem & pi : candidates )
  {
    xmlPrintOtherUpdateOn( cout, pi );
  }
}

// ----------------------------------------------------------------------------

// returns true if NEEDED! restartSuggested() patches are available
static bool list_patch_updates( Zypper & zypper, bool all_r, const PatchSelector &sel, const PatchHistoryData & historyData_r )
{
  Table tbl;
  FillPatchesTable intoTbl( tbl, historyData_r );

  Table pmTbl; // only NEEDED! that affect packagemanager (restartSuggested()), they have priority
  FillPatchesTable intoPMTbl( pmTbl, historyData_r );

  PatchCheckStats stats( zypper.config().exclude_optional_patches );
  CliMatchPatch cliMatchPatch( zypper, sel._requestedPatchDates, sel._requestedPatchCategories, sel._requestedPatchSeverity );

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

    if ( all_r || patchIsApplicable( pi ) )
    {
      if ( ! all_r && patchIsNeededRestartSuggested( pi ) )
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
find_updates( const ResKind & kind, Candidates & candidates, bool all_r )
{
  const ResPool& pool = God->pool();
  DBG << "Looking for update candidates of kind " << kind << endl;

  // package updates
  if (kind == ResKind::package && !all_r)
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
find_updates(const ResKindSet & kinds, Candidates & candidates , bool all_r)
{
  for (ResKindSet::const_iterator kit = kinds.begin(); kit != kinds.end(); ++kit)
    find_updates( *kit, candidates, all_r );

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

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort, bool all_r, const PatchSelector &patchSel_r )
{
  PatchHistoryData patchHistoryData;	// commonly used by all tables

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
      affects_pkgmgr = xml_list_patches( zypper, all_r, patchHistoryData );
    else
    {
      if (kinds.size() > 1)
      {
        zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL);
        zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      }
      affects_pkgmgr = list_patch_updates( zypper, all_r, patchSel_r, patchHistoryData );
    }
    localkinds.erase(it);
  }

  // list other kinds (only if there are no _patches_ affecting the package manager)

  // XML output here
  if (zypper.out().type() == Out::TYPE_XML)
  {
    if (!affects_pkgmgr)
    {
      xml_list_updates( localkinds, all_r );
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
    bool hide_repo = best_effort || InitRepoSettings::instance()._repoFilter.size();

    // header
    TableHeader th;
    unsigned name_col;
    // TranslatorExplanation S stands for Status
    th << N_("S");
    if (!hide_repo)
      th << N_("Repository");

    name_col = th.cols();
    th << N_("Name");
    // best_effort does not know version or arch yet
    if (!best_effort)
    {
      if (*it == ResKind::package)
        th << table::EditionStyleSetter( tbl, N_("Current Version") );
      th << table::EditionStyleSetter( tbl, N_("Available Version") ) << N_("Arch");
    }

    tbl << th;

    unsigned cols = th.cols();

    ResPoolProxy uipool( ResPool::instance().proxy() );

    Candidates candidates;
    find_updates( *it, candidates, all_r );

    for ( const PoolItem & pi : candidates )
    {
      TableRow tr (cols);
      tr << computeStatusIndicator( pi );
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
void list_patches_by_issue( Zypper & zypper, bool all_r, const PatchSelector & sel_r )
{
  PatchHistoryData patchHistoryData;	// commonly used by all tables

  // Overall CLI filter
  bool only_needed = !all_r;
  CliMatchPatch cliMatchPatch( zypper,
                               sel_r._requestedPatchDates,
                               sel_r._requestedPatchCategories,
                               sel_r._requestedPatchSeverity );


  // pass1 finding PoolItems and their matching issues (pi,itype,iid)
  std::vector<const Issue*> pass2; // on the fly remember anyType issues for pass2
  std::map<PoolItem,std::map<std::string,std::set<std::string>>> iresult;
  for ( const Issue & issue : sel_r._requestedIssues )
  {
    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive( false );
    q.addKind( ResKind::patch );
    // PoolQuery ORs attributes but we need AND.
    // Post processing the match must assert correct type of specific IDs!
    if ( issue.specificType() && issue.anyId() )
    { q.addAttribute(sat::SolvAttr::updateReferenceType, issue.type() ); }
    else
    {
      q.addAttribute( sat::SolvAttr::updateReferenceId, issue.id() );
      if ( issue.anyType() && issue.specificId() ) 				// remember for pass2
      {
        q.addAttribute( sat::SolvAttr::updateReferenceType, issue.id() );	// bnc#941309: let '--issue=bugzilla' also match the type
        pass2.push_back( &issue );
      }
    }

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi { *it };

      if ( only_needed && ! patchIsApplicable( pi ) )
        continue;

      if ( ! cliMatchPatch( pi ) )
      {
        DBG << pi.ident() << " skipped. (not matching CLI filter)" << endl;
        continue;
      }

      for_( d, it.matchesBegin(), it.matchesEnd() )
      {
        std::string itype { d->subFind( sat::SolvAttr::updateReferenceType ).asString() };
        if ( issue.specificType() && itype != issue.type() )
          continue;	// assert correct type of specific IDs
        // remember....
        iresult[std::move(pi)][std::move(itype)].insert( d->subFind( sat::SolvAttr::updateReferenceId ).asString() );
      }
    }
  }

  //pass2 (summary/description)
  std::vector<PoolItem> dresult;	// query will assert unique entries
  for ( const Issue * _issue : pass2 )
  {
    const Issue & issue { *_issue };
    PoolQuery q;
    q.setMatchSubstring();
    q.setCaseSensitive( false );
    q.addKind( ResKind::patch );
    q.addAttribute(sat::SolvAttr::summary, issue.id() );
    q.addAttribute(sat::SolvAttr::description, issue.id() );

    for_( it, q.begin(), q.end() )
    {
      PoolItem pi { *it };

      if ( only_needed && ! patchIsApplicable( pi ) )
        continue;

      if ( ! cliMatchPatch( pi ) )
      {
        DBG << pi.ident() << " skipped. (not matching CLI filter)" << endl;
        continue;
      }

      if ( ! iresult.count( pi ) )
      { dresult.push_back( pi ); }
    }
  }

  ///////////////////////////////////////////////////////////////////
  // print result
  if (zypper.out().type() == Out::TYPE_XML)
  {
    xmlout::Node parent { cout, "list-patches-byissue", xmlout::Node::optionalContent };
    xmlPrintPatchUpdateListOn( *parent, "issue-matches", zypp_pending::make_map_key_Iterable( iresult ), patchHistoryData );
    xmlPrintPatchUpdateListOn( *parent, "description-matches", dresult, patchHistoryData );
  }
  else
  {
    // iresult to table
    Table issueMatchesTbl;
    FillPatchesTableForIssue intoIssueMatchesTbl( issueMatchesTbl, patchHistoryData );
    for ( const auto & res : iresult )
    {
      const PoolItem & pi { res.first };
      for ( const auto & ires : res.second )
      {
        const std::string & itype { ires.first };
        for ( const std::string & iid : ires.second )
        { intoIssueMatchesTbl( pi, itype, iid ); }
      }
    }

    // dresult to table
    Table descrMatchesTbl;
    FillPatchesTable intoDescrMatchesTbl( descrMatchesTbl, patchHistoryData );
    for ( const auto & pi : dresult )
    {
      intoDescrMatchesTbl( pi );
    }


    if ( issueMatchesTbl.empty() && descrMatchesTbl.empty() )
    {  zypper.out().info(_("No matching issues found.")); }
    else
    {
      if ( !issueMatchesTbl.empty() )
      {
        zypper.out().gap();
        zypper.out().info(_("The following matches in issue numbers have been found:"));

        issueMatchesTbl.sort(); // use default sort
        zypper.out().gap();
        cout << issueMatchesTbl;
      }

      if ( !descrMatchesTbl.empty() )
      {
        zypper.out().gap();
        zypper.out().info(_( "Matches in patch descriptions of the following patches have been found:"));

        descrMatchesTbl.sort(); // use default sort
        zypper.out().gap();
        cout << descrMatchesTbl;
      }
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates_by_issue( Zypper & zypper, const std::set<Issue> &issues, SolverRequester::Options srOpts )
{
  for ( const Issue & issue : issues )
  {
    PoolQuery q;
    q.setMatchExact();
    q.setCaseSensitive( false );
    q.addKind( ResKind::patch );
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
