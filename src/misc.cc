/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>

#include <zypp/ZYppFactory.h>
#include <zypp/base/Logger.h>

#include <zypp/SrcPackage.h>
#include <zypp/Package.h>
#include <zypp/Capabilities.h>
#include <zypp/ui/Selectable.h>

#include <zypp/RepoInfo.h>

#include <zypp/PoolQuery.h>
#include <zypp/PoolItemBest.h>

#include "Zypper.h"
#include "main.h"
#include "utils/misc.h"
#include "utils/pager.h"
#include "utils/prompt.h"
#include "utils/getopt.h"
#include "utils/richtext.h"

#include "misc.h"

extern ZYpp::Ptr God;

void remove_selections( Zypper & zypper )
{
  // zypp gets initialized only upon the first successful processing of
  // command options, if the command was not the 'help'. bnc #372696
  if ( !God )
    return;

  MIL << "Removing user selections from the solver pool" << endl;
  DBG << "Removing user setToBeInstalled()/Removed()" << endl;

  // iterate pool, searching for ResStatus::isByUser()
  // TODO optimize: remember user selections and iterate by name
  // TODO optimize: it seems this is actually needed only if the selection was
  //      not committed (user has chosen not to continue)
  const ResPool & pool( God->pool() );

  for ( ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it )
    if ( it->status().isByUser() )
    {
      DBG << "Removing user setToBeInstalled()/Removed()" << endl;
      it->status().resetTransact( ResStatus::USER );
    }

  DBG << "Removing user addRequire() addConflict()" << endl;

  Resolver_Ptr solver = God->resolver();
  // FIXME port this
//   CapSet capSet = solver->getConflict();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing conflict: " << (*it) << endl;
//     solver->removeConflict(*it);
//   }
//   capSet = solver->getRequire();
//   for (CapSet::const_iterator it = capSet.begin(); it != capSet.end(); ++it)
//   {
//     DBG << "removing require: " << (*it) << endl;
//     solver->removeRequire(*it);
//   }

  MIL << "DONE" << endl;
}

// ----------------------------------------------------------------------------
namespace
{
  inline std::string get_display_name( const PoolItem & obj )
  {
    // in most cases we want to display full product name (bnc #589333)
    if ( obj.isKind<Product>() )
      return obj.summary();
    return obj.name();
  }
} // namespace

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
bool confirm_licenses( Zypper & zypper )
{
  bool confirmed = true;
  bool license_auto_agree = zypper.cOpts().count("auto-agree-with-licenses")
                         || zypper.cOpts().count("agree-to-third-party-licenses");

  for ( const PoolItem & pi : God->pool() )
  {
    bool to_accept = true;

    if ( pi.status().isToBeInstalled() && !pi.licenseToConfirm().empty() )
    {
      ui::Selectable::Ptr selectable = God->pool().proxy().lookup( pi.kind(), pi.name() );

      // this is an upgrade, check whether the license changed
      // for now we only do dumb string comparison (bnc #394396)
      if ( selectable->hasInstalledObj() )
      {
        bool differ = false;
        for_( inst, selectable->installedBegin(), selectable->installedEnd() )
	  if ( inst->resolvable()->licenseToConfirm() != pi.licenseToConfirm() )
          { differ = true; break; }

        if ( !differ )
        {
          DBG << "old and new license does not differ for " << pi.name() << endl;
          continue;
        }
        DBG << "new license for " << pi.name() << " is different, needs confirmation " << endl;
      }

      if ( license_auto_agree )
      {
	zypper.out().info(
	  // translators: the first %s is name of the resolvable,
	  // the second is its kind (e.g. 'zypper package')
	  str::Format(_("Automatically agreeing with %s %s license."))
	  % get_display_name( pi )
	  % kind_to_string_localized(pi.kind(),1) );

        MIL << "Automatically agreeing with " << pi.name() << " " <<  pi.kind() << " license." << endl;
        continue;
      }

      std::ostringstream s;
      std::string kindstr;
      if ( pi.kind() != ResKind::package )
	kindstr = " (" + kind_to_string_localized( pi.kind(), 1 ) + ")";

      if ( !pi.needToAcceptLicense() )
        to_accept = false;

      if (to_accept)
      {
        // introduction
	// translators: the first %s is the name of the package, the second
	// is " (package-type)" if other than "package" (patch/product/pattern)
	s << str::form(_("In order to install '%s'%s, you must agree to terms of the following license agreement:"),
		       get_display_name( pi ).c_str(), kindstr.c_str());
	s << endl << endl;
      }
      // license text
      printRichText( s, pi.licenseToConfirm() );

      // show in pager unless we are read by a machine or the pager fails
      if ( zypper.globalOpts().machine_readable || !show_text_in_pager( s.str() ) )
        zypper.out().info( s.str(), Out::QUIET );

      if ( to_accept )
      {
        // lincense prompt
        std::string question( _("Do you agree with the terms of the license?") );
        //! \todo add 'v' option to view the license again, add prompt help
        if ( !read_bool_answer( PROMPT_YN_LICENSE_AGREE, question, license_auto_agree ) )
        {
          confirmed = false;

          if ( zypper.globalOpts().non_interactive )
          {
            zypper.out().info(_("Aborting installation due to the need for license confirmation."), Out::QUIET );
	    zypper.out().info(
	      // translators: %s is '--auto-agree-with-licenses'
	      str::Format(_("Please restart the operation in interactive mode and confirm your agreement with required licenses, or use the %s option."))
	      % "--auto-agree-with-licenses", Out::QUIET );

            MIL << "License(s) NOT confirmed (non-interactive without auto confirmation)" << endl;
          }
          else
          {
	    zypper.out().info(
	      // translators: e.g. "... with flash package license."
	      str::Format(_("Aborting installation due to user disagreement with %s %s license."))
	      % get_display_name( pi )
	      % kind_to_string_localized( pi.kind(), 1 ), Out::QUIET );
	    MIL << "License(s) NOT confirmed (interactive)" << endl;
	  }
          break;
        }
      }
    }
  }
  return confirmed;
}

// ----------------------------------------------------------------------------

void report_licenses( Zypper & zypper )
{
  PoolQuery q;
  unsigned count_installed = 0, count_installed_repo = 0, count_installed_eula = 0;
  std::set<std::string> unique_licenses;

  for ( ui::Selectable::constPtr s : q.selectable() )
  {
    if ( !s )  // FIXME this must not be necessary!
      continue;

    for ( const PoolItem & inst : s->installed() )
    {
      ++count_installed;

      cout
        << s->name() << "-" << inst.edition()
        << " (" << kind_to_string_localized( s->kind(), 1 ) << ")"
        << endl;

      if ( s->kind() == ResKind::package )
      {
        cout
          << _("License") << ": "
          << asKind<Package>(inst)->license()
          << endl;
        unique_licenses.insert( asKind<Package>(inst)->license() );
      }

      PoolItem inst_with_repo;
      for ( const PoolItem & api : s->available() )
      {
        if ( identical(api, inst) )
        {
          inst_with_repo = api;
          ++count_installed_repo;
          break;
        }
      }

      if ( inst_with_repo && !inst_with_repo.licenseToConfirm().empty() )
      {
        cout << _("EULA") << ":" << endl;
	printRichText( cout, inst_with_repo.licenseToConfirm() );
        cout << endl;

        ++count_installed_eula;
      }
      else if ( !inst.licenseToConfirm().empty() )
        cout << "look! got an installed-only item and it has EULA! he?" << inst << endl;
      cout << "-" << endl;
    }
  }

  cout << endl << _("SUMMARY") << endl << endl;
  cout << str::form(_("Installed packages: %d"), count_installed) << endl;
  cout << str::form(_("Installed packages with counterparts in repositories: %d"), count_installed_repo) << endl;
  cout << str::form(_("Installed packages with EULAs: %d"), count_installed_eula) << endl;

  cout << str::form("Package licenses (%u):", (unsigned) unique_licenses.size()) << endl;
  for_( it, unique_licenses.begin(), unique_licenses.end() )
    cout << "* " << *it << endl;
}

// ----------------------------------------------------------------------------
namespace
{
  SrcPackage::constPtr source_find( Zypper & zypper_r, const std::string & arg_r )
  {
    /*
     * Workflow:
     *
     * 1. return srcpackage "arg_r" if available
     * 2. else if package "arg_r" is available, return it's srcpackage if available
     * 3; else return 0
     */
    DBG << "looking for source package: " << arg_r << endl;
    ui::Selectable::Ptr p( ui::Selectable::get( ResKind::srcpackage, arg_r ) );
    if ( p )
      return asKind<SrcPackage>( p->theObj().resolvable() );

    // else: try package and packages sourcepackage
    p = ui::Selectable::get( ResKind::package, arg_r );
    if ( p )
    {
      std::string name( p->theObj()->asKind<Package>()->sourcePkgName() );
      DBG << "looking for source package of package: " << name << endl;
      zypper_r.out().info( str::Format(_("Package '%s' has source package '%s'.")) % arg_r % name );

      p = ui::Selectable::get( ResKind::srcpackage, p->theObj()->asKind<Package>()->sourcePkgName() );
      if ( p )
	return asKind<SrcPackage>( p->theObj().resolvable() );
      else
	zypper_r.out().error( str::Format(_("Source package '%s' for package '%s' not found.")) % name % arg_r );
    }
    else
      zypper_r.out().error( str::Format(_("Source package '%s' not found.")) % arg_r );


    DBG << "no source package found for: " << arg_r << endl;
    return SrcPackage::constPtr();
  }
} // namespace
// ----------------------------------------------------------------------------

void build_deps_install( Zypper & zypper )
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for ( const std::string & arg : zypper.arguments() )
  {
    SrcPackage::constPtr srcpkg = source_find( zypper, arg );

    if ( srcpkg )
    {
      DBG << "Injecting build requieres for " << srcpkg << endl;

      // install build depenendcies only
      if ( zypper.cOpts().count("build-deps-only") )
        for_( itc, srcpkg->requires().begin(), srcpkg->requires().end() )
        {
          God->resolver()->addRequire( *itc );
          DBG << "requiring: " << *itc << endl;
        }
      // install the source package with build deps
      else
      {
        Capability cap( srcpkg->name(), Rel::EQ, srcpkg->edition(), ResKind::srcpackage );
        God->resolver()->addRequire( cap );
        DBG << "requiring: " << cap << endl;
      }
    }
    else if ( !zypper.globalOpts().ignore_unknown )
    {
      zypper.setExitCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    }
  }
}

// ----------------------------------------------------------------------------

void mark_src_pkgs( Zypper & zypper )
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for ( const std::string & arg : zypper.arguments() )
  {
    SrcPackage::constPtr srcpkg = source_find( zypper, arg );

    if ( srcpkg )
      zypper.runtimeData().srcpkgs_to_install.insert( srcpkg );
  }
}

// ----------------------------------------------------------------------------

void install_src_pkgs( Zypper & zypper )
{
  for_( it, zypper.runtimeData().srcpkgs_to_install.begin(), zypper.runtimeData().srcpkgs_to_install.end() )
  {
    SrcPackage::constPtr srcpkg = *it;
    zypper.out().info( str::Format(_("Installing source package %s-%s")) % srcpkg->name() % srcpkg->edition() );
    MIL << "Going to install srcpackage: " << srcpkg << endl;

    try
    {
      if ( get_download_option( zypper, true ) == DownloadOnly )
      {
        God->provideSrcPackage( srcpkg ).resetDispose();

        zypper.out().info( str::Format(_("Source package %s-%s successfully retrieved.")) % srcpkg->name() % srcpkg->edition() );
      }
      else
      {
        God->installSrcPackage( srcpkg );

        zypper.out().info( str::Format(_("Source package %s-%s successfully installed.")) % srcpkg->name() % srcpkg->edition() );
      }
    }
    catch ( const Exception & ex )
    {
      ZYPP_CAUGHT( ex );
      zypper.out().error( ex, str::Format(_("Problem installing source package %s-%s:")) % srcpkg->name() % srcpkg->edition() );
      zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
