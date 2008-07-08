/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <sstream>
#include <boost/format.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"

#include "zypp/SrcPackage.h"
#include "zypp/Capabilities.h"

#include "zypp/RepoInfo.h"

#include "main.h"
#include "utils.h"
#include "getopt.h"
#include "richtext.h"
#include "prompt.h"

#include "misc.h"
#include "utils/pager.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;

// converts a user-supplied kind to a zypp kind object
// returns an empty one if not recognized
ResKind string_to_kind (const string &skind)
{
  ResObject::Kind empty;
  string lskind = str::toLower (skind);
  if (lskind == "package")
    return ResTraits<Package>::kind;
  if (lskind == "pattern")
    return ResTraits<Pattern>::kind;
  if (lskind == "product")
    return ResTraits<Product>::kind;
  if (lskind == "patch")
    return ResTraits<Patch>::kind;
  if (lskind == "script")
    return ResTraits<Script>::kind;
  if (lskind == "message")
    return ResTraits<Message>::kind;
  if (lskind == "atom")
    return ResTraits<Atom>::kind;
//   if (lskind == "system")
//     return ResTraits<SystemResObject>::kind;
  if (lskind == "srcpackage")
    return ResTraits<SrcPackage>::kind;
  // not recognized
  return empty;
}

// ----------------------------------------------------------------------------

void remove_selections(Zypper & zypper)
{
  // zypp gets initialized only upon the first successful processing of
  // command options, if the command was not the 'help'. bnc #372696
  if (!God)
    return;

  MIL << "Removing user selections from the solver pool" << endl;

  DBG << "Removing user setToBeInstalled()/Removed()" << endl;

  // iterate pool, searching for ResStatus::isByUser()
  // TODO optimize: remember user selections and iterate by name
  // TODO optimize: it seems this is actually needed only if the selection was
  //      not committed (user has chosen not to continue)
  const ResPool & pool = God->pool();

  for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it)
    if (it->status().isByUser())
    {
      DBG << "Removing user setToBeInstalled()/Removed()" << endl;
      it->status().resetTransact(zypp::ResStatus::USER);
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

/* debugging
static
ostream& operator << (ostream & stm, ios::iostate state)
{
  return stm << (state & ifstream::eofbit ? "Eof ": "")
	     << (state & ifstream::badbit ? "Bad ": "")
	     << (state & ifstream::failbit ? "Fail ": "")
	     << (state == 0 ? "Good ": "");
}
*/

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
bool confirm_licenses(Zypper & zypper)
{
  bool confirmed = true;

  for (ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it)
  {
    if (it->status().isToBeInstalled() &&
        !it->resolvable()->licenseToConfirm().empty())
    {
      if (zypper.cmdOpts().license_auto_agree)
      {
      	zypper.out().info(boost::str(
            // TranslatorExplanation The first %s is name of the resolvable, the second is its kind (e.g. 'zypper package')
      	    format(_("Automatically agreeing with %s %s license."))
            % it->resolvable()->name()
            % kind_to_string_localized(it->resolvable()->kind(),1)));

        MIL << format("Automatically agreeing with %s %s license.")
            % it->resolvable()->name() % it->resolvable()->kind().asString()
            << endl;

        continue;
      }

      ostringstream s;
      string kindstr =
        it->resolvable()->kind() != ResKind::package ?
          " (" + kind_to_string_localized(it->resolvable()->kind(), 1) + ")" :
          string();

      // introduction
      s << str::form(
          // translators: the first %s is the name of the package, the second
          // is " (package-type)" if other than "package" (patch/product/pattern)
          _("In order to install '%s'%s, you must agree"
            " to terms of the following license agreement:"),
            it->resolvable()->name().c_str(), kindstr.c_str());
      s << endl << endl;

      // license text
      const string& licenseText = it->resolvable()->licenseToConfirm();
      if (licenseText.find("DT:Rich")==licenseText.npos)
        s << licenseText;
      else
        s << processRichText(licenseText);

      // show in pager unless we are read by a machine or the pager fails
      if (zypper.globalOpts().machine_readable || !show_in_pager(s.str()))
        zypper.out().info(s.str(), Out::QUIET);

      // lincense prompt
      string question = _("Do you agree with the terms of the license?");
      //! \todo add 'v' option to view the license again, add prompt help
      if (!read_bool_answer(PROMPT_YN_LICENSE_AGREE, question, zypper.cmdOpts().license_auto_agree))
      {
        confirmed = false;

        if (zypper.globalOpts().non_interactive)
        {
          zypper.out().info(
            _("Aborting installation due to the need for license confirmation."),
            Out::QUIET);
          zypper.out().info(boost::str(format(
            // translators: %sanslate the '--auto-agree-with-licenses',
            // it is a command line option
            _("Please restart the operation in interactive"
              " mode and confirm your agreement with required licenses,"
              " or use the %s option.")) % "--auto-agree-with-licenses"),
            Out::QUIET);

          MIL << "License(s) NOT confirmed (non-interactive without auto confirmation)" << endl;
        }
        else
        {
          zypper.out().info(boost::str(format(
              // translators: e.g. "... with flash package license."
              //! \todo fix this to allow proper translation
              _("Aborting installation due to user disagreement with %s %s license."))
                % it->resolvable()->name()
                % kind_to_string_localized(it->resolvable()->kind(), 1)),
              Out::QUIET);
            MIL << "License(s) NOT confirmed (interactive)" << endl;
        }

        break;
      }
    }
  }

  return confirmed;
}

static SrcPackage::constPtr source_find( const string & arg )
{
   /*
   * Workflow:
   *
   * 1. interate all SrcPackage resolvables with specified name
   * 2. find the latest version or version satisfying specification.
   */
    SrcPackage::constPtr srcpkg;

    ResPool pool(God->pool());
    DBG << "looking source for : " << arg << endl;
    for_( srcit, pool.byIdentBegin<SrcPackage>(arg), 
              pool.byIdentEnd<SrcPackage>(arg) )
    {
      DBG << *srcit << endl;
      if ( ! srcit->status().isInstalled() ) // this will be true for all of the srcpackages, won't it?
      {
        SrcPackage::constPtr _srcpkg = asKind<SrcPackage>(srcit->resolvable());

        DBG << "Considering srcpakcage " << _srcpkg->name() << "-" << _srcpkg->edition() << ": ";
        if (srcpkg)
        {
          if (_srcpkg->edition() > srcpkg->edition())
          {
            DBG << "newer edition (" << srcpkg->edition() << " > " << _srcpkg->edition() << ")";
            _srcpkg.swap(srcpkg);
          }
          else
            DBG << "is older than the current candidate";
        }
        else
        {
          DBG << "first candindate";
          _srcpkg.swap(srcpkg);
        }
        DBG << endl;
      }
    }

    return srcpkg;
}

void build_deps_install(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
    {
      DBG << format("Injecting build requieres for source package %s-%s")
          % srcpkg->name() % srcpkg->edition() << endl;

      // add all src requires to pool DEPRECATED: srcpakcages will be in
      // the pool (together with their build-deps) like normal packages
      // so only require the srcpackage
      /*
      for_( itc, srcpkg->dep(Dep::REQUIRES).begin(), srcpkg->dep(Dep::REQUIRES).end() )
      {
        God->resolver()->addRequire(*itc);
        DBG << "added req: " << *itc << endl;
      }*/
      God->resolver()->addRequire(Capability(srcpkg->name(), Rel::EQ, srcpkg->edition(), ResTraits<SrcPackage>::kind));
      //installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    }
    else
    {
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    }
  }
}


void find_src_pkgs(Zypper & zypper)
{
  /*
   * Workflow:
   *
   * 1. find the latest version or version satisfying specification.
   * 2. install the source package with ZYpp->installSrcPackage(SrcPackage::constPtr);
   */

  for (vector<string>::const_iterator it = zypper.arguments().begin();
       it != zypper.arguments().end(); ++it)
  {
    SrcPackage::constPtr srcpkg = source_find(*it);

    if (srcpkg)
      zypper.runtimeData().srcpkgs_to_install.push_back(srcpkg);
    else
      zypper.out().error(boost::str(format(
          _("Source package '%s' not found.")) % (*it)));
  }
}

void install_src_pkgs(Zypper & zypper)
{
  for_(it, zypper.runtimeData().srcpkgs_to_install.begin(), zypper.runtimeData().srcpkgs_to_install.end())
  {
    SrcPackage::constPtr srcpkg = *it;
    zypper.out().info(boost::str(format(
        _("Installing source package %s-%s"))
        % srcpkg->name() % srcpkg->edition()));
    MIL << "Going to install srcpackage: " << srcpkg << endl;
  
    try
    {
      God->installSrcPackage(srcpkg);
  
      zypper.out().info(boost::str(format(
          _("Source package %s-%s successfully installed."))
          % srcpkg->name() % srcpkg->edition()));
    }
    catch (const Exception & ex)
    {
      ZYPP_CAUGHT(ex);
      zypper.out().error(ex,
        boost::str(format(_("Problem installing source package %s-%s:"))
          % srcpkg->name() % srcpkg->edition()));
  
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
