/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/


/** \file SolverRequester.cc
 *
 */
#include <zypp/ZYppFactory.h>
#include <zypp/base/LogTools.h>

#include <zypp/PoolQuery.h>
#include <zypp/PoolItemBest.h>

#include <zypp/Capability.h>
#include <zypp/Resolver.h>
#include <zypp/Patch.h>
#include <zypp/ui/Selectable.h>

#include "Zypper.h"
#include "SolverRequester.h"

// libzypp logger settings
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper:req"

/////////////////////////////////////////////////////////////////////////
namespace
{
  void getCiMatchHint( PoolQuery & q_r, std::string & ciMatchHint_r )
  {
    q_r.setCaseSensitive( false );
    if ( ! q_r.empty() )
    {
      unsigned cnt = 0;
      for_( it, q_r.selectableBegin(), q_r.selectableEnd() )
      {
	if ( cnt == 3 )
	{
	  ciMatchHint_r += ",...";
	  break;
	}
	else
	{
	  if ( cnt )
	    ciMatchHint_r += ", ";
	  ciMatchHint_r += (*it)->name();
	}
	++cnt;
      }
    }
  }

  PoolQuery pkg_spec_to_poolquery( const Capability & cap, const std::list<std::string> & repos )
  {
    //
    sat::Solvable::SplitIdent splid( cap.detail().name() );

    PoolQuery q;
    q.setMatchGlob();
    q.setCaseSensitive( true );
    q.addKind( splid.kind() );
    for_( it, repos.begin(), repos.end() )
      q.addRepo( *it );
    q.addDependency( sat::SolvAttr::name, splid.name().asString(),
		     // only package names (no provides)
		     cap.detail().op(), cap.detail().ed(),
		     // defaults to Rel::ANY (NOOP) if no versioned cap
		     Arch( cap.detail().arch() ) );
    // defaults Arch_empty (NOOP) if no arch in cap

    DBG << "query: " << q << endl;
    return q;
  }

  PoolQuery pkg_spec_to_poolquery( const Capability & cap, const std::string & repo )
  {
    std::list<std::string> repos;
    if ( !repo.empty() )
      repos.push_back( repo );
    return pkg_spec_to_poolquery( cap, repos );
  }

  std::set<PoolItem> get_installed_providers( const Capability & cap )
  {
    std::set<PoolItem> providers;

    sat::WhatProvides q( cap );
    for_( it, q.poolItemBegin(), q.poolItemEnd() )
    {
      if ( traits::isPseudoInstalled( (*it).satSolvable().kind() ) )
      {
	if ( (*it).isSatisfied() )
	  providers.insert( *it );
      }
      else if ( (*it).satSolvable().isSystem() )
	providers.insert( *it );
    }
    return providers;
  }

  PoolItem get_installed_obj( ui::Selectable::Ptr & s )
  {
    PoolItem installed;
    if ( traits::isPseudoInstalled( s->kind() ) )
    {
      for_( it, s->availableBegin(), s->availableEnd() )
	// this is OK also for patches - isSatisfied() excludes !isRelevant()
	if ( it->status().isSatisfied() && ( !installed || installed->edition() < (*it)->edition() ) )
	  installed = *it;
    }
    else
      installed = s->installedObj();

    return installed;
  }

} // namespace
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// SolverRequester::Options
/////////////////////////////////////////////////////////////////////////

void SolverRequester::Options::setForceByCap( bool value )
{
  if ( value && force_by_name )
    DBG << "resetting previously set force_by_name" << endl;

  force_by_cap = value;
  force_by_name = !force_by_cap;
}

void SolverRequester::Options::setForceByName( bool value )
{
  if ( value && force_by_cap )
    DBG << "resetting previously set force_by_cap" << endl;

  force_by_name = value;
  force_by_cap = !force_by_name;
}


/////////////////////////////////////////////////////////////////////////
// SolverRequester
/////////////////////////////////////////////////////////////////////////

void SolverRequester::install( const PackageArgs & args )
{
  _command = ZypperCommand::INSTALL;
  installRemove( args );
}

// ----------------------------------------------------------------------------

void SolverRequester::remove( const PackageArgs & args )
{
  _command = ZypperCommand::REMOVE;
  if ( args.options().do_by_default )
  {
    INT << "PackageArgs::Options::do_by_default == true."
        << " Set it to 'false' when doing 'remove'" << endl;
    return;
  }

  installRemove( args );
}

// ----------------------------------------------------------------------------

void SolverRequester::installRemove( const PackageArgs & args )
{
  if ( args.empty() )
    return;

  for_( it, args.dos().begin(), args.dos().end() )
    install( *it );

  // TODO solve before processing dontCaps? so that we could unset any
  // dontCaps that are already set for installation. This would allow
  //   $ zypper install pattern:lamp_sever -someunwantedpackage
  // and similar nice things.
  // ma: use weaklocks on them!
  for_( it, args.donts().begin(), args.donts().end() )
    remove( *it );
}

// ----------------------------------------------------------------------------

/*
 * For given Capability & repo & Options:
 *
 * 1) if --capability option was not specified, try to install 'by name' first.
 *    I.e. via ui::Selectable and/or PoolItem.
 *    note: wildcards must be supported here, use PoolQuery with match_glob
 *          to find the selectables
 *
 * 2) if no package could be found by name and --name was not specified,
 *    or --capability was specified, install 'by capability',
 *    i.e. using ResPool::addRequires(cap)
 *
 * NOTES
 * - In both cases check for already installed packages, and if found, hand over
 *   to \ref updateTo(const Capability&, const string&, const PoolItem&) method.
 * - If the requested command was UPDATE and the object is not installed,
 *   do no action other than report the fact. The is the only difference between
 *   install and update.
 * - If the argument contains repository, issue request forcing the repo
 *
 * TODO
 * - maybe a check for glob wildcards in cap name would make sense before trying
 *   by-cap
 */
void SolverRequester::install( const PackageSpec & pkg )
{
  std::string ciMatchHint;	// hint on possible typo (case-insensitive matches)

  // first try by name
  if ( !_opts.force_by_cap )
  {
    PoolQuery q;
    if ( !pkg.repo_alias.empty() )
      q = pkg_spec_to_poolquery( pkg.parsed_cap, pkg.repo_alias );
    else
      q = pkg_spec_to_poolquery( pkg.parsed_cap, _opts.from_repos );

    // get the best matching items and tag them for installation.
    // FIXME this ignores vendor lock - we need some way to do --from which
    // would respect vendor lock: e.g. a new Selectable::updateCandidateObj(Options&)
    PoolItemBest bestMatches( q.begin(), q.end() );

    if ( !bestMatches.empty() )
    {
      unsigned notInstalled = 0;
      for_( sit, bestMatches.begin(), bestMatches.end() )
      {
        ui::Selectable::Ptr s( ui::asSelectable()( *sit ) );
        if ( s->kind() == ResKind::patch )
          installPatch( pkg, *sit );
        else
        {
          PoolItem instobj = get_installed_obj( s );
          if ( instobj )
          {
            if ( s->availableEmpty() )
            {
              if ( !_opts.force )
                addFeedback( Feedback::ALREADY_INSTALLED, pkg, instobj, instobj );
              addFeedback( Feedback::NOT_IN_REPOS, pkg, instobj, instobj );
              MIL << s->name() << " not in repos, can't (re)install" << endl;
              return;
            }

            // whether user requested specific repo/version/arch
            bool userconstraints = pkg.parsed_cap.detail().isVersioned()
	                        || pkg.parsed_cap.detail().hasArch()
				|| !_opts.from_repos.empty()
				|| !pkg.repo_alias.empty();

            // check vendor (since PoolItemBest does not do it)
            bool changes_vendor = ! VendorAttr::instance().equivalent( instobj->vendor(), (*sit)->vendor() );

            PoolItem best;
            if ( userconstraints )
              updateTo( pkg, *sit);
            else if  (_opts.force )
              updateTo( pkg, s->highestAvailableVersionObj() );
            else if ( (best = s->updateCandidateObj()) )
              updateTo( pkg, best );
            else if ( changes_vendor && !_opts.allow_vendor_change )
              updateTo( pkg, instobj );
            else
              updateTo( pkg, *sit );
          }
          else if ( _command == ZypperCommand::INSTALL )
          {
            setToInstall( *sit );
            MIL << "installing " << *sit << endl;
          }
          else
	  {
	    ++notInstalled;
            // addFeedback(Feedback::NOT_INSTALLED, pkg);
	    // delay Feedback::NOT_INSTALLED until we know
	    // there is not a single match installed.
	  }
        }
      }
      if ( notInstalled == bestMatches.size() )
      {
	addFeedback( Feedback::NOT_INSTALLED, pkg );
      }
      return;
    }
    else if ( _opts.force_by_name || pkg.modified )
    {
      addFeedback( Feedback::NOT_FOUND_NAME, pkg );
      WAR << pkg << " not found" << endl;
      return;
    }

    addFeedback( Feedback::NOT_FOUND_NAME_TRYING_CAPS, pkg );
    // Quick check whether there would have been matches with different case..
    getCiMatchHint( q, ciMatchHint );
  }

  // try by capability
  // is there a provider for the requested capability?
  sat::WhatProvides q( pkg.parsed_cap );
  if ( q.empty() )
  {
    addFeedback( Feedback::NOT_FOUND_CAP, pkg, ciMatchHint );
    WAR << pkg << " not found" << endl;
    return;
  }

  // is the provider already installed?
  std::set<PoolItem> providers = get_installed_providers( pkg.parsed_cap );
  // already installed, try to update()
  for_( it, providers.begin(), providers.end() )
  {
    if ( _command == ZypperCommand::INSTALL )
      addFeedback( Feedback::ALREADY_INSTALLED, pkg, *it, *it );
    MIL << "provider '" << *it << "' of '" << pkg.parsed_cap << "' installed" << endl;
  }

  if ( providers.empty() )
  {
    DBG << "adding requirement " << pkg.parsed_cap << endl;
    addRequirement( pkg );
  }
}

// ----------------------------------------------------------------------------

/**
 * Remove packages based on given Capability & Options from the system.
 */
void SolverRequester::remove( const PackageSpec & pkg )
{
  std::string ciMatchHint;	// hint on possible typo (case-insensitive matches)

  // first try by name
  if ( !_opts.force_by_cap )
  {
    PoolQuery q = pkg_spec_to_poolquery( pkg.parsed_cap, "" );

    if ( !q.empty() )
    {
      bool got_installed = false;
      for_( it, q.poolItemBegin(), q.poolItemEnd() )
      {
        if ( it->status().isInstalled() )
        {
          DBG << "Marking for deletion: " << *it << endl;
          setToRemove( *it );
          got_installed = true;
        }
      }
      if ( got_installed )
        return;
      else
      {
        addFeedback( Feedback::NOT_INSTALLED, pkg );
        MIL << "'" << pkg.parsed_cap << "' is not installed" << endl;
        if ( _opts.force_by_name)
          return;
      }
      // TODO handle patches (cannot uninstall!), patterns (remove content(?))
    }
    else if ( _opts.force_by_name || pkg.modified )
    {
      addFeedback( Feedback::NOT_FOUND_NAME, pkg );
      WAR << pkg << "' not found" << endl;
      return;
    }

    addFeedback( Feedback::NOT_FOUND_NAME_TRYING_CAPS, pkg );
    // Quick check whether there would have been matches with different case..
    getCiMatchHint( q, ciMatchHint );
  }

  // try by capability
  // is there a provider for the requested capability?
  sat::WhatProvides q( pkg.parsed_cap );
  if ( q.empty() )
  {
    addFeedback( Feedback::NOT_FOUND_CAP, pkg, ciMatchHint );
    WAR << pkg << " not found" << endl;
    return;
  }

  // is the provider already installed?
  std::set<PoolItem> providers = get_installed_providers( pkg.parsed_cap );

  // not installed, nothing to do
  if ( providers.empty() )
  {
    addFeedback( Feedback::NO_INSTALLED_PROVIDER, pkg );
    MIL << "no provider of " << pkg.parsed_cap << "is installed" << endl;
  }
  else
  {
    MIL << "adding conflict " << pkg.parsed_cap << endl;
    addConflict( pkg );
  }
}

// ----------------------------------------------------------------------------

void SolverRequester::update(const PackageArgs & args)
{
  if (args.empty() )
    return;

  _command = ZypperCommand::UPDATE;

  for_( it, args.dos().begin(), args.dos().end() )
    install( *it );
}

// ----------------------------------------------------------------------------

void SolverRequester::updatePatterns()
{ /*NOOP*/ }

// ----------------------------------------------------------------------------

void SolverRequester::updatePatches()
{
  DBG << "going to mark needed patches for installation" << endl;

  // search twice: if there are none with restartSuggested(), retry on all
  // unless --updatestack-only.
  // (in the first run, ignore_pkgmgmt == 0, in the second it is 1)
  bool any_marked = false;
  bool dateLimit = ( _opts.cliMatchPatch._dateBefore != Date() );
  for ( unsigned ignore_pkgmgmt = 0; !any_marked && ignore_pkgmgmt < 2; ++ignore_pkgmgmt )
  {
    for ( const auto & selPtr : getZYpp()->pool().proxy().byKind( ResKind::patch ) )
    {
      PackageSpec patch;
      patch.orig_str = selPtr->name();
      patch.parsed_cap = Capability(selPtr->name());

      // bnc#919709: a date limit must ignore newer patch candidates
      PoolItem candidateObj( selPtr->candidateObj() );
      if ( dateLimit && asKind<Patch>(candidateObj)->timestamp() > _opts.cliMatchPatch._dateBefore )
      {
	for ( const auto & pi : selPtr->available() )
	{
	  if ( asKind<Patch>(pi)->timestamp() <= _opts.cliMatchPatch._dateBefore )
	  {
	    candidateObj = pi;
	    break;
	  }
	}
      }

      if ( installPatch( patch, candidateObj, ignore_pkgmgmt ) )
        any_marked = true;
    }

    if ( ! ignore_pkgmgmt )	// just checked the update stack
    {
      if ( any_marked )
	MIL << "got some pkgmgmt patches, will install these first" << endl;

      if ( Zypper::instance()->cOpts().count("updatestack-only") )
      {
	MIL << "updatestack-only: will stop here!" << endl;
	break;
      }
    }
  }
}

// ----------------------------------------------------------------------------

bool SolverRequester::installPatch( const PoolItem & selected )
{
  PackageSpec patchspec;
  patchspec.orig_str = str::form("%s-%s", selected->name().c_str(), selected->edition().asString().c_str() );
  patchspec.parsed_cap = Capability( selected->name(), Rel::EQ, selected->edition(), ResKind::patch );

  return installPatch( patchspec, selected );
}

// ----------------------------------------------------------------------------

bool SolverRequester::installPatch( const PackageSpec & patchspec, const PoolItem & selected, bool ignore_pkgmgmt )
{
  Patch::constPtr patch = asKind<Patch>(selected);

  if ( selected.status().isBroken() ) // bnc #506860
  {
    DBG << "Needed candidate patch " << patch
        << " affects_pkgmgmt: " << patch->restartSuggested()
        << (ignore_pkgmgmt ? " (ignored)" : "") << endl;

    if ( ignore_pkgmgmt || patch->restartSuggested() )
    {
      Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
      if ( Zypper::instance()->globalOpts().reboot_req_non_interactive )
        ignoreFlags |= Patch::Reboot;
      if ( Zypper::instance()->cOpts().count("auto-agree-with-licenses")
	|| Zypper::instance()->cOpts().count("agree-to-third-party-licenses") )
	ignoreFlags |= Patch::License;

      if ( selected.isUnwanted() )
      {
        DBG << "candidate patch " << patch << " is locked" << endl;
        addFeedback( Feedback::PATCH_UNWANTED, patchspec, selected, selected );
        return false;
      }

      if ( _opts.skip_optional_patches && patch->categoryEnum() == Patch::CAT_OPTIONAL )
      {
        DBG << "candidate patch " << patch << " is optional" << endl;
        addFeedback( Feedback::PATCH_OPTIONAL, patchspec, selected, selected );
	return false;
      }

      // bnc #221476
      if ( _opts.skip_interactive && patch->interactiveWhenIgnoring( ignoreFlags ) )
      {
        addFeedback( Feedback::PATCH_INTERACTIVE_SKIPPED, patchspec, selected );
        return false;
      }

      {
	CliMatchPatch::Missmatch missmatch = _opts.cliMatchPatch.missmatch( patch );
	if ( missmatch != CliMatchPatch::Missmatch::None )
	{
	  Feedback::Id id = Feedback::INVALID_REQUEST;
	  switch ( missmatch )
	  {
	    case CliMatchPatch::Missmatch::Date:	id = Feedback::PATCH_TOO_NEW;	break;
	    case CliMatchPatch::Missmatch::Category:	id = Feedback::PATCH_WRONG_CAT;	break;
	    case CliMatchPatch::Missmatch::Severity:	id = Feedback::PATCH_WRONG_SEV;	break;
	    case CliMatchPatch::Missmatch::None:	/* make gcc happy */		break;
	  }
	  DBG << "candidate patch " << patch << " does not pass CLI filter (" << static_cast<unsigned>(missmatch) << ")" << endl;
	  addFeedback( id, patchspec, selected, selected );
	  return false;
	}
      }

      // passed:
      // TODO use _opts.force
      setToInstall( selected );
      MIL << "installing " << selected << endl;
      return true;
    }
  }
  else if ( selected.status().isSatisfied() )
  {
    if ( _command == ZypperCommand::INSTALL || _command == ZypperCommand::UPDATE )
    {
      DBG << "candidate patch " << patch << " is already satisfied" << endl;
      addFeedback( Feedback::ALREADY_INSTALLED, patchspec, selected, selected );
    }
  }
  else
  {
    if ( _command == ZypperCommand::INSTALL || _command == ZypperCommand::UPDATE )
    {
      DBG << "candidate patch " << patch << " is irrelevant" << endl;
      addFeedback( Feedback::PATCH_NOT_NEEDED, patchspec, selected );
    }
  }

  return false;
}

// ----------------------------------------------------------------------------

void SolverRequester::updateTo( const PackageSpec & pkg, const PoolItem & selected )
{
  if ( !selected )
  {
    INT << "Candidate is empty, returning! Pass PoolItem you want to update to." << endl;
    return;
  }

  ui::Selectable::Ptr s = ui::asSelectable()( selected );

  // the best object without repository, arch, or version restriction
  PoolItem theone = s->updateCandidateObj();
  // the best installed object
  PoolItem installed = get_installed_obj( s );
  // highest available version
  PoolItem highest = s->highestAvailableVersionObj();

  if ( !installed )
  {
    INT << "no installed object, nothing to update, returning" << endl;
    return;
#warning TODO handle pseudoinstalled objects
  }

  DBG << "selected:  " << selected << endl;
  DBG << "best:      " << theone    << endl;
  DBG << "highest:   " << highest   << endl;
  DBG << "installed: " << installed << endl;


  // ******* request ********
  bool action = true;
  if ( !identical( installed, selected ) || _opts.force )
  {
    if ( _opts.best_effort )
    {
      // require version greater than than the one installed
      Capability c( s->name(), Rel::GT, installed->edition(), s->kind() );
      PackageSpec pkg;
      pkg.orig_str = s->name();
      pkg.parsed_cap = c;
      addRequirement( pkg );
      MIL << *s << " update: adding requirement " << c << endl;
    }
    else if ( selected->edition() > installed->edition() )
    {
      // set 'candidate' for installation
      setToInstall(selected);
      MIL << *s << " update: setting " << selected << " to install" << endl;
    }
    else if ( selected->edition() == installed->edition()
	    && selected->arch() != installed->arch()
	    && pkg.parsed_cap.detail().hasArch() /*userselected architecture*/ )
    {
      // set 'candidate' for installation
      setToInstall( selected );
      MIL << *s << " update: setting " << selected << " to install (arch change request)" << endl;
    }
    else if ( selected->edition() == installed->edition()
	    && !pkg.repo_alias.empty() /*userselected repo*/ )
    {
      // set 'candidate' for installation
      setToInstall( selected );
      MIL << *s << " update: setting " << selected << " to install (repo change request)" << endl;
    }
    else if ( _opts.force || _opts.oldpackage )
    {
      // set 'candidate' for installation
      setToInstall( selected );
      MIL << *s << " update: forced setting " << selected << " to install" << endl;
    }
    else
      action = false;
  }
  else
    action = false;


  // ******* report ********

  // the candidate is already installed
  if ( identical( installed, selected ) || ( !action && installed->edition() == selected->edition() ) )
  {
    if ( _opts.force )
      return;

    // only say 'already installed' in case of install, if update was requested
    // only report if we fail to install the newest version (the code below)
    if ( _command == ZypperCommand::INSTALL )
    {
      addFeedback( Feedback::ALREADY_INSTALLED, pkg, selected, installed );
      MIL << "'" << pkg.parsed_cap << "'";
      if ( !pkg.repo_alias.empty() )
        MIL << " from '" << pkg.repo_alias << "'";
      MIL << " already installed." << endl;
    }
    // TODO other kinds

    // no available object (bnc #591760)
    // !availableEmpty() <=> theone && highest
    if ( s->availableEmpty() )
    {
      addFeedback( Feedback::NO_UPD_CANDIDATE, pkg, PoolItem(), installed );
      DBG << "no available objects in repos, skipping update of " << s->name() << endl;
      return;
    }

    // the highest version is already there
    if ( identical( installed, highest ) || highest->edition() <= installed->edition() )
      addFeedback( Feedback::NO_UPD_CANDIDATE, pkg, selected, installed );
  }
  else if ( installed->edition() > selected->edition() )
  {
    if ( _opts.force || _opts.oldpackage )
      return;

    addFeedback( Feedback::SELECTED_IS_OLDER, pkg, selected, installed );
    MIL << "Selected is older than the installed. Will not downgrade unless --oldpackage is used" << endl;
  }


  // there is higher version available than the selected candidate
  // this can happen because of repo priorities, locks, vendor lock, and
  // because of conditions given on comm. line: repos/arch/version (bnc #522223)
  if ( highest           // should not happen, but just in case (bnc #607482 c#4)
    && !identical( selected, highest )
    && highest->edition() > installed->edition() )
  {
    // whether user requested specific repo/version/arch
    bool userconstraints = pkg.parsed_cap.detail().isVersioned()
			|| pkg.parsed_cap.detail().hasArch()
			|| !_opts.from_repos.empty()
			|| !pkg.repo_alias.empty();
    if ( userconstraints )
    {
      addFeedback( Feedback::UPD_CANDIDATE_USER_RESTRICTED, pkg, selected, installed );
      DBG << "Newer object exists, but has different repo/arch/version: " << highest << endl;
    }

    // update candidate locked
    if ( s->status() == ui::S_Protected || highest.status().isLocked() )
    {
      addFeedback( Feedback::UPD_CANDIDATE_IS_LOCKED, pkg, selected, installed );
      DBG << "Newer object exists, but is locked: " << highest << endl;
    }

    // update candidate has different vendor
    if ( !VendorAttr::instance().equivalent( highest->vendor(), installed->vendor() )
      && !_opts.allow_vendor_change )
    {
      addFeedback( Feedback::UPD_CANDIDATE_CHANGES_VENDOR, pkg, selected, installed );
      DBG << "Newer object with different vendor exists: " << highest
          << " (" << highest->vendor() << ")"
          << ". Installed vendor: " << installed->vendor() << endl;
    }

    // update candidate is from low-priority (higher priority number) repo
    if ( highest->repoInfo().priority() > selected->repoInfo().priority() )
    {
      addFeedback( Feedback::UPD_CANDIDATE_HAS_LOWER_PRIO, pkg, selected, installed );
      DBG << "Newer object exists in lower-priority repo: " << highest << endl;
    }
  } // !identical(selected, highest) && highest->edition() > installed->edition()
}

// ----------------------------------------------------------------------------

void SolverRequester::setToInstall( const PoolItem & pi )
{
  if ( _opts.force )
  {
    pi.status().setToBeInstalled( ResStatus::USER );
    addFeedback( Feedback::FORCED_INSTALL, PackageSpec(), pi );
  }
  else if ( ui::asSelectable()(pi)->locked() )
  {
    // Workaround: Use a solver request instead of selecting the item.
    // This will enable the solver to report the lock conflict, while
    // selecting the item will silently remove the lock.
    // Basically the right way but zypp solver job API needs polishing
    // as ther are better jobs than 'addRequire'.
    sat::Solvable solv( pi.satSolvable() );
    Capability cap( solv.arch(), solv.name(), Rel::EQ, solv.edition(), solv.kind() );
    getZYpp()->resolver()->addRequire( cap );
    _requires.insert( cap );
    addFeedback( Feedback::SET_TO_INSTALL, PackageSpec(), pi );
    addFeedback( Feedback::INSTALLED_LOCKED, PackageSpec(), pi );
    return;
  }
  else
  {
    ui::asSelectable()(pi)->setOnSystem( pi, ResStatus::USER );
    addFeedback( Feedback::SET_TO_INSTALL, PackageSpec(), pi );
  }
  _toinst.insert( pi );
}

// ----------------------------------------------------------------------------

void SolverRequester::setToRemove( const PoolItem & pi )
{
  if ( ui::asSelectable()(pi)->locked() )
  {
    // Workaround: Use a solver request instead of selecting the item.
    // This will enable the solver to report the lock conflict, while
    // selecting the item will silently remove the lock.
    // Basically the right way but zypp solver job API needs polishing
    // as ther are better jobs than 'addRequire'.
    sat::Solvable solv( pi.satSolvable() );
    Capability cap( solv.arch(), solv.name(), Rel::EQ, solv.edition(), solv.kind() );
    getZYpp()->resolver()->addConflict( cap );
    _conflicts.insert( cap );
    addFeedback( Feedback::SET_TO_REMOVE, PackageSpec(), pi );
    addFeedback( Feedback::INSTALLED_LOCKED, PackageSpec(), pi );
    return;
  }
  pi.status().setToBeUninstalled( ResStatus::USER );
  addFeedback( Feedback::SET_TO_REMOVE, PackageSpec(), pi );
  _toremove.insert( pi );
}

// ----------------------------------------------------------------------------

void SolverRequester::addRequirement( const PackageSpec & pkg )
{
  getZYpp()->resolver()->addRequire( pkg.parsed_cap );
  addFeedback( Feedback::ADDED_REQUIREMENT, pkg );
  _requires.insert( pkg.parsed_cap );
}

// ----------------------------------------------------------------------------

void SolverRequester::addConflict( const PackageSpec & pkg )
{
  getZYpp()->resolver()->addConflict( pkg.parsed_cap );
  addFeedback( Feedback::ADDED_CONFLICT, pkg );
  _conflicts.insert( pkg.parsed_cap );
}

// ----------------------------------------------------------------------------

bool SolverRequester::hasFeedback( const Feedback::Id id ) const
{
  for_( fb, _feedback.begin(), _feedback.end() )
    if ( fb->id() == id )
      return true;
  return false;
}
