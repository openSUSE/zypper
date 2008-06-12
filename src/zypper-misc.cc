#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <boost/format.hpp>
#include <sys/wait.h> //for wait()

#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Algorithm.h"

#include "zypp/Edition.h"
#include "zypp/Patch.h"
#include "zypp/Package.h"
#include "zypp/SrcPackage.h"
#include "zypp/Capabilities.h"
#include "zypp/PoolQuery.h"
#include "zypp/TmpPath.h"


#include "zypp/media/MediaException.h"
#include "zypp/FileChecker.h"

#include "zypp/RepoInfo.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"
#include "zypper-getopt.h"
#include "zypper-richtext.h"
#include "zypper-prompt.h"
#include "output/prompt.h"

#include "zypper-misc.h"

using namespace std;
using namespace zypp;
using namespace boost;

extern ZYpp::Ptr God;
extern RuntimeData gData;


/**
 * Loops through resolvables, checking if there is license to confirm. When
 * run interactively, it displays a dialog, otherwise it answers automatically
 * according to --auto-agree-with-licenses present or not present.
 * 
 * \returns true if all licenses have been confirmed, false otherwise.  
 */
static bool confirm_licenses(Zypper & zypper);

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

// copied from yast2-pkg-bindings:PkgModuleFunctions::DoProvideNameKind
bool ProvideProcess::operator()( const PoolItem& provider )
{
  DBG << "Considering " << provider << endl;
  // 1. compatible arch
  // 2. best arch
  // 3. best edition

  // check the repository alias if it's specified
  if (!_repo.empty() && _repo != provider->repository().info().alias())
  {
    DBG << format ("Skipping repository %s (requested: %s)")
      % provider->repository().info().alias() % _repo << endl;
    return true;
  }

  bool is_installed;
  if (provider->isKind(ResKind::package))
    is_installed = provider.status().isInstalled();
  else
    is_installed = provider.isSatisfied();

  if (!is_installed)
  {
    // deselect the item if it's already selected,
    // only one item should be selected
    if (provider.status().isToBeInstalled()) {
      DBG << "  Deselecting" << endl;
      provider.status().resetTransact(whoWantsIt);
    }

    // regarding items which are installable only
    if (!provider->arch().compatibleWith( _architecture )) {
      DBG << format ("provider %s has incompatible arch '%s'")
        % provider->name() % provider->arch().asString() << endl;
    }
    else if (!item) {
      DBG << "  First match" << endl;
      item = provider;
    }
    else if (item->arch().compare( provider->arch() ) < 0) {
      DBG << "  Better arch" << endl;
      item = provider;
    }
    else if (item->edition().compare( provider->edition() ) < 0) {
      DBG << "  Better edition" << endl;
      item = provider;
    }
  }
  else {
    // store encountered target item (installed)
    installed_item = provider;
    if (!item) item = provider;
  }

  return true;
}

string pager_help_exit(const string &pager)
{
  string endfour = pager.substr(pager.size()-4,4);
  if (endfour == "less")
  {
    return str::form(_("Press '%c' to exit the pager."), 'q');
  }
  return string();
}

string pager_help_navigation(const string &pager)
{
  string endfour = pager.substr(pager.size()-4,4);
  if (endfour == "less")
  {
    return _("Use arrows or pgUp/pgDown keys to scroll the text by lines or pages.");
  }
  else if (endfour == "more")
  {
    return _("Use the Enter or Space key to scroll the text by lines or pages.");
  }
  return string();
}

//gets true if successfully display in pager
bool show_in_pager(const string& text)
{
  const char* envpager = getenv("PAGER");
  if (!envpager)
    envpager="more"; //basic posix default, must be in PATH
  string pager(envpager);
  filesystem::TmpFile tfile;
  string tpath = tfile.path().absolutename().c_str();
  ofstream os(tpath.c_str());

  string help = pager_help_navigation(pager);
  if (!help.empty())
    os << "(" << help << ")" << endl << endl;
  os << text;

  help = pager_help_exit(pager);
  if (!help.empty())
    os << endl << endl << "(" << help << ")";
  os.close();

  ostringstream cmdline;
  cmdline << pager <<" "<<tpath;

  switch(fork())
  {
  case -1:
    WAR << "fork failed" << endl;
    return false;

  case 0:
    execlp("sh","sh","-c",cmdline.str().c_str(),(char *)0);
    WAR << "exec failed with " << strerror(errno) << endl;
    exit(1); // cannot return false here, because here is another process
             // so only kill myself
             //! \todo FIXME proper exit code, message?

  default: 
    wait(0); //wait until pager end to disallow possibly terminal collision
  }

  return true;
}

// this does only resolvables with this _name_.
// we could also act on _provides_
// TODO edition, arch
static void mark_for_install(Zypper & zypper,
                      const ResObject::Kind &kind,
		      const std::string &name,
		      const std::string & repo = "")
{
  // name and kind match:
  ProvideProcess installer (ZConfig::instance().systemArchitecture(), repo);
  DBG << "Iterating over [" << kind << "]" << name << endl;
  invokeOnEach(
      God->pool().byIdentBegin(kind, name),
      God->pool().byIdentEnd(kind, name),
      zypp::functor::functorRef<bool,const zypp::PoolItem&> (installer));

  DBG << "... done" << endl;
  if (!installer.item)
  {
    zypper.out().error(
      // translators: meaning a package %s or provider of capability %s
      str::form(_("'%s' not found"), name.c_str()));
    WAR << str::form("'%s' not found", name.c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return;
  }

  if (installer.installed_item &&
      installer.installed_item.resolvable()->edition() == installer.item.resolvable()->edition() &&
      installer.installed_item.resolvable()->arch() == installer.item.resolvable()->arch() &&
      ( ! copts.count("force") ) )
  {
    // if it is broken install anyway, even if it is installed
    if ( installer.item.isBroken() )
    {
      installer.item.status().setTransact( true, zypp::ResStatus::USER );
    }

    zypper.out().info(boost::str(format(
      // translators: e.g. skipping package 'zypper' (the newest version already installed)  
      _("skipping %s '%s' (the newest version already installed)"))
      % kind_to_string_localized(kind,1) % name));
  }
  else {

    //! \todo don't use setToBeInstalled for this purpose but higher level solver API
    bool result = installer.item.status().setToBeInstalled( zypp::ResStatus::USER );
    if (!result)
    {
      // this is because the resolvable is installed and we are forcing.
      installer.item.status().setTransact( true, zypp::ResStatus::USER );
      if (!copts.count("force"))
      {
        zypper.out().error(boost::str(
          format(_("Failed to add '%s' to the list of packages to be installed."))
          % name));
        ERR << "Could not set " << name << " as to-be-installed" << endl;
      }
    }
  }
}

struct DeleteProcess
{
  bool found;
  DeleteProcess ()
    : found(false)
  { }

  bool operator() ( const PoolItem& provider )
  {
    found = true;
    DBG << "Marking for deletion: " << provider << endl;
    bool result = provider.status().setToBeUninstalled( zypp::ResStatus::USER );
    if (!result) {
      Zypper::instance()->out().error(boost::str(format(
          _("Failed to add '%s' to the list of packages to be removed."))
          % provider.resolvable()->name()));
      ERR << "Could not set " << provider.resolvable()->name()
          << " as to-be-uninstalled" << endl;
    }
    return true;		// get all of them
  }
};

// mark all matches
static void mark_for_uninstall(Zypper & zypper,
                        const ResObject::Kind &kind,
			const std::string &name)
{
  const ResPool &pool = God->pool();
  // name and kind match:

  DeleteProcess deleter;
  DBG << "Iterating over " << name << endl;
  invokeOnEach( pool.byIdentBegin( kind, name ),
		pool.byIdentEnd( kind, name ),
		resfilter::ByInstalled(),
		zypp::functor::functorRef<bool,const zypp::PoolItem&> (deleter)
		);
  DBG << "... done" << endl;
  if (!deleter.found) {
    zypper.out().error(
      // translators: meaning a package %s or provider of capability %s
      str::form(_("'%s' not found"), name.c_str()));
    WAR << str::form("'%s' not found", name.c_str()) << endl;
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return;
  }
}


static void
mark_by_name (Zypper & zypper,
              bool install_not_remove,
              const ResObject::Kind &kind,
              const string &name,
              const string & repo = "")
{
  if (install_not_remove)
    mark_for_install(zypper, kind, name, repo);
  else
    mark_for_uninstall(zypper, kind, name);
}


// don't try NAME-EDITION yet, could be confused by
// dbus-1-x11, java-1_4_2-gcj-compat, ...
/*
bool mark_by_name_edition (...)
  static const regex rx_name_edition("(.*?)-([0-9].*)");

  smatch m;
  if (! is_cap && regex_match (capstr, m, rx_name_edition)) {
    capstr = m.str(1) + " = " + m.str(2);
    is_cap = true;
  }
*/

static void
mark_by_capability (Zypper & zypper,
                    bool install_not_remove,
                    const ResKind & kind,
                    const Capability & cap)
{
  if (!cap.empty()) {
    DBG << "Capability: " << cap << endl;

    Resolver_Ptr resolver = zypp::getZYpp()->resolver();
    if (install_not_remove) {
      DBG << "Adding requirement " << cap << endl;
      resolver->addRequire (cap);
    }
    else {
      DBG << "Adding conflict " << cap << endl;
      resolver->addConflict (cap);
    }
  }
}

// ----------------------------------------------------------------------------

// join arguments at comparison operators ('=', '>=', and the like)
static void
install_remove_preprocess_args(const Zypper::ArgList & args,
                               Zypper::ArgList & argsnew)
{
  Zypper::ArgList::size_type argc = args.size();
  argsnew.reserve(argc);
  string tmp;
  // preprocess the arguments
  for(Zypper::ArgList::size_type i = 0, lastnew = 0; i < argc; ++i)
  {
    tmp = args[i];
    if (i
        && (tmp == "=" || tmp == "==" || tmp == "<"
            || tmp == ">" || tmp == "<=" || tmp == ">=")
        && i < argc - 1)
    {
      argsnew[lastnew-1] += tmp + args[++i];
      continue;
    }
    else if (tmp.find_last_of("=<>") == tmp.size() - 1 && i < argc - 1)
    {
      argsnew.push_back(tmp + args[++i]);
      ++lastnew;
    }
    else if (i && tmp.find_first_of("=<>") == 0)
    {
      argsnew[lastnew-1] += tmp;
      ++i;
    }
    else
    {
      argsnew.push_back(tmp);
      ++lastnew;
    }
  }

  DBG << "old: ";
  copy(args.begin(), args.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl << "new: ";
  copy(argsnew.begin(), argsnew.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl;
}

static void
mark_selectable(Zypper & zypper,
                ui::Selectable & s,
                bool install_not_remove,
                bool force,
                const string & repo = "",
                const string & arch = "")
{
  PoolItem theone = s.theObj();
  //! \todo handle multiple installed case
  bool theoneinstalled; // is the One installed ?
  if (s.kind() == ResKind::package)
    theoneinstalled = !s.installedEmpty() && theone &&
      equalNVRA(*s.installedObj().resolvable(), *theone.resolvable());
  else if (s.kind() == ResKind::patch)
    theoneinstalled = theone.isRelevant() && theone.isSatisfied();
  else
    theoneinstalled = theone.isSatisfied();

  bool anyinstalled = theoneinstalled;
  PoolItem installed;
  if (theoneinstalled)
    installed = theone;
  else
  {
    if (s.kind() == ResKind::package)
    {
      anyinstalled = s.hasInstalledObj();
      installed = s.installedObj();
    }
    else if (s.kind() == ResKind::patch)
    {
      for_(it, s.availableBegin(), s.availableEnd())
      {
        if (it->isRelevant() && it->isSatisfied())
        {
          if (installed)
          {
            if (it->resolvable()->edition() > installed->edition())
              installed = *it;
          }
          else
          {
            installed = *it;
            anyinstalled = true;
          }
        }
      }
    }
    else
    {
      for_(it, s.availableBegin(), s.availableEnd())
      {
        if (it->status().isSatisfied())
        {
          if (installed)
          {
            if (it->resolvable()->edition() > installed->edition())
              installed = *it;
          }
          else
          {
            installed = *it;
            anyinstalled = true;
          }
        }
      }
    }
  }

  if (install_not_remove)
  {
    if (theoneinstalled && !force)
    {
      DBG << "the One (" << theone << ") is installed, skipping." << endl;
      zypper.out().info(str::form(
          _("'%s' is already installed."), s.name().c_str()));
      return;
    }

    if (theoneinstalled && force)
    {
      s.setStatus(ui::S_Install);
      DBG << s << " install: forcing reinstall" << endl;
    }
    else
    {
      Capability c;

      // require version greater than the installed. The solver should pick up
      // the latest installable in this case, which is what we want for all
      // kinds (and for patches having the latest installed should automatically
      // satisfy all older version, or make them irrelevant)

      //! could be a problem for patches if there would a greater version
      //! of a patch appear that would be irrelevant at the same time. Should
      //! happen probably.
      if (anyinstalled)
        c = Capability(s.name(), Rel::GT, installed->edition(), s.kind());
      // require any version
      else
        c = Capability(s.name(), s.kind());

      God->resolver()->addRequire(c);
      DBG << s << " install: adding requirement " << c << endl;
    }
  }
  // removing is simpler, as usually
  //! \todo but not that simple - simply adding a conflict with a pattern
  //! or patch does not make the packages it requires to be removed.
  //! we still need to define what response of zypper -t foo bar should be for PPP
  else
  {
    if (!anyinstalled)
    {
      zypper.out().info(str::form(
          _("'%s' is not installed."), s.name().c_str()));
      DBG << s << " remove: not installed, skipping." << endl;
      return;
    }

    Capability c(s.name(), s.kind());
    God->resolver()->addConflict(c);
    DBG << s << " remove: adding conflict " << c << endl;
  }
}

// ----------------------------------------------------------------------------

void install_remove(Zypper & zypper,
                    const Zypper::ArgList & args,
                    bool install_not_remove,
                    const ResKind & kind)
{
  if (args.empty())
    return;
  
  bool force_by_capability = zypper.cOpts().count("capability");
  bool force_by_name = zypper.cOpts().count("name");
  bool force = zypper.cOpts().count("force");
  if (force)
    force_by_name = true;

  if (force_by_capability && force_by_name)
  {
    zypper.out().error(boost::str(
      format(_("%s contradicts %s")) % "--capability" % (force? "--force" : "--name")));
    
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException());
  }

  if (install_not_remove && force_by_capability && force)
  {
    // translators: meaning --force with --capability
    zypper.out().error(boost::str(format(_("%s cannot currently be used with %s"))
      % "--force" % "--capability"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
    ZYPP_THROW(ExitRequestException());
  }

  Zypper::ArgList argsnew;
  install_remove_preprocess_args(args, argsnew);

  string str, arch, repo;
  bool by_capability;
  const ResPool & pool = God->pool();
  for_(it, argsnew.begin(), argsnew.end())
  {
    str = *it; arch.clear(); repo.clear();
    by_capability = false;

    // install remove modifiers
    if (str[0] == '+' || str[0] == '~')
    {
      install_not_remove = true;
      str.erase(0, 1);
    }
    else if (str[0] == '-' || str[0] == '!')
    {
      install_not_remove = false;
      str.erase(0, 1);
    }

    string::size_type pos;

    if ((pos = str.rfind(':')) != string::npos)
    {
      repo = str.substr(0, pos);
      str = str.substr(pos + 1);
      force_by_name = true; // until there is a solver API for this
    }

    //! \todo force arch with '.' or '@'???
    if ((pos = str.find('.')) != string::npos)
    { }

    // mark by name by force
    if (force_by_name)
    {
      mark_by_name (zypper, install_not_remove, kind, str, repo);
      continue;
    }

    // recognize missplaced command line options given as packages
    // bnc#391644
    if ( str[0] == '-' )
    {
      // FIXME show a message here, after string freeze
      //zypper.out().error(boost::str(format(_("%s is not a valid package or capability name.") % str));
      zypper.out().error(_("No valid arguments specified."));
      zypper.setExitCode(ZYPPER_EXIT_ERR_INVALID_ARGS);
      ZYPP_THROW(ExitRequestException());
    }
    

    // is version specified?
    by_capability = str.find_first_of("=<>") != string::npos;

    // try to find foo-bar-1.2.3-2
    if (!by_capability && str.find('-') != string::npos)
    {
      // try to find the original string first as name
      // search by name, not in whatprovides, since there can be provider even
      // for cracklib-dict=small
      // continue only if nothing has been found this way
      if (pool.byIdentBegin(kind, str) == pool.byIdentEnd(kind, str))
      {
        // try to replace '-' for '=' from right to the left and check
        // whether there is something providing such capability
        string::size_type pos = string::npos;
        while ((pos = str.rfind('-', pos)) != string::npos)
        {
          string trythis = str;
          trythis.replace(pos, 1, 1, '=');
          string tryver = str.substr(pos + 1, str.size() - 1);

          DBG << "trying: " << trythis << " edition: " << tryver << endl;

          Capability cap = safe_parse_cap (zypper, trythis, kind);
          sat::WhatProvides q(cap);
          for_(sit, q.begin(), q.end())
          {
            if (sit->edition().match(tryver) == 0)
            {
              str = trythis;
              by_capability = true;
              DBG << str << "might be what we wanted" << endl;
              break;
            }
          }
          if (by_capability)
            break;
          --pos;
        }
      }
    }

    // try to find by name + wildcards first
    if (!by_capability)
    {
      PoolQuery q;
      q.addKind(kind);
      q.addAttribute(sat::SolvAttr::name, str);
      q.setMatchGlob();
      bool found = false;
      for_(s, q.selectableBegin(), q.selectableEnd())
      {
        mark_selectable(zypper, **s, install_not_remove, force);
        found = true;
      }
      // done with this requirement, skip to next argument
      if (found)
        continue;
    }

    // try by capability

    Capability cap = safe_parse_cap (zypper, str, kind);
    sat::WhatProvides q(cap);

    // is there a provider for the requested capability?
    if (q.empty())
    {
      // translators: meaning a package %s or provider of capability %s
      zypper.out().error(str::form(_("'%s' not found."), str.c_str()));
      WAR << str::form("'%s' not found", str.c_str()) << endl;
      zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
      continue;
    }

    // is the provider already installed?
    bool installed = false;
    string provider; 
    for_(solvit, q.poolItemBegin(), q.poolItemEnd())
    {
      if (solvit->resolvable()->isKind(ResKind::package))
        installed = solvit->status().isInstalled();
      else
        installed = solvit->isSatisfied();
      if (installed)
      {
        provider = solvit->resolvable()->name(); 
        break;
      }
    }
    // already installed, nothing to do
    if (installed && install_not_remove)
    {
      if (provider == str)
        zypper.out().info(str::form(
            _("'%s' is already installed."), str.c_str()));
      else
        zypper.out().info(str::form(
            // translators: %s are package names
            _("'%s' providing '%s' is already installed."),
            provider.c_str(), str.c_str()));

      MIL << str::form("skipping '%s': already installed", str.c_str()) << endl;
      continue;
    }
    // not installed, nothing to do
    else if (!installed && !install_not_remove)
    {
      // translators: meaning a package %s or provider of capability %s
      zypper.out().info(str::form(_("'%s' is not installed."), str.c_str()));
      MIL << str::form("skipping '%s': not installed", str.c_str()) << endl;
      continue;
    }

    mark_by_capability (zypper, install_not_remove, kind, cap);
  }
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

//! @return true to retry solving now, false to cancel, indeterminate to continue
static tribool show_problem (Zypper & zypper,
                      const ResolverProblem & prob,
                      ProblemSolutionList & todo)
{
  ostringstream desc_stm;
  string tmp;
  // translators: meaning 'dependency problem' found during solving
  desc_stm << _("Problem: ") << prob.description () << endl;
  tmp = prob.details ();
  if (!tmp.empty ())
    desc_stm << "  " << tmp << endl;

  int n;
  ProblemSolutionList solutions = prob.solutions ();
  ProblemSolutionList::iterator
    bb = solutions.begin (),
    ee = solutions.end (),
    ii;
  for (n = 1, ii = bb; ii != ee; ++n, ++ii) {
    // TranslatorExplanation %d is the solution number
    desc_stm << format (_(" Solution %d: ")) % n << (*ii)->description () << endl;
    tmp = (*ii)->details ();
    if (!tmp.empty ())
      desc_stm << indent(tmp, 2) << endl;
  }

  unsigned int problem_count = God->resolver()->problems().size();
  unsigned int solution_count = solutions.size(); 

  // without solutions, its useless to prompt
  if (solutions.empty())
  {
    zypper.out().error(desc_stm.str());
    return false;
  }

  string prompt_text;
  if (problem_count > 1)
    prompt_text = _PL(
      "Choose the above solution using '1' or skip, retry or cancel",
      "Choose from above solutions by number or skip, retry or cancel",
      solution_count);
  else
    prompt_text = _PL(
      // translators: translate 'c' to whatever you translated the 'c' in
      // "c" and "s/r/c" strings
      "Choose the above solution using '1' or cancel using 'c'",
      "Choose from above solutions by number or cancel",
      solution_count);

  PromptOptions popts;
  unsigned int default_reply;
  ostringstream numbers;
  for (unsigned int i = 1; i <= solution_count; i++)
    numbers << i << "/";

  if (problem_count > 1)
  {
    default_reply = solution_count + 2;
    // translators: answers for dependency problem solution input prompt:
    // "Choose from above solutions by number or skip, retry or cancel"
    // Translate the letters to whatever is suitable for your language.
    // The anserws must be separated by slash characters '/' and must
    // correspond to skip/retry/cancel in that order.
    // The answers should be lower case letters.
    popts.setOptions(numbers.str() + _("s/r/c"), default_reply);
  }
  else
  {
    default_reply = solution_count;
    // translators: answers for dependency problem solution input prompt:
    // "Choose from above solutions by number or cancel"
    // Translate the letter 'c' to whatever is suitable for your language
    // and to the same as you translated it in the "s/r/c" string
    // See the "s/r/c" comment for other details.
    // One letter string  for translation can be tricky, so in case of problems,
    // please report a bug against zypper at bugzilla.novell.com, we'll try to solve it.
    popts.setOptions(numbers.str() + _("c"), default_reply);
  }

  zypper.out().prompt(PROMPT_DEP_RESOLVE, prompt_text, popts, desc_stm.str());
  unsigned int reply =
    get_prompt_reply(zypper, PROMPT_DEP_RESOLVE, popts);

  // retry
  if (problem_count > 1 && reply == solution_count + 1)
    return true;
  // cancel (one problem)
  if (problem_count == 1 && reply == solution_count)
    return false;
  // cancel (more problems)
  if (problem_count > 1 && reply == solution_count + 2)
    return false;
  // skip
  if (problem_count > 1 && reply == solution_count)
    return indeterminate; // continue with next problem

  zypper.out().info(boost::str(format (_("Applying solution %s")) % (reply + 1)), Out::HIGH);
  ProblemSolutionList::iterator reply_i = solutions.begin ();
  advance (reply_i, reply);
  todo.push_back (*reply_i);

  tribool go_on = indeterminate; // continue with next problem
  return go_on;
}

// return true to retry solving, false to cancel transaction
static bool show_problems(Zypper & zypper)
{
  bool retry = true;
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();
  ResolverProblemList rproblems = resolver->problems ();
  ResolverProblemList::iterator
    b = rproblems.begin (),
    e = rproblems.end (),
    i;
  ProblemSolutionList todo;

  // display the number of problems
  if (rproblems.size() > 1)
    zypper.out().info(boost::str(format(
      _PL("%d Problem:", "%d Problems:", rproblems.size())) % rproblems.size()));
  else if (rproblems.empty())
  {
    // should not happen! If solve() failed at least one problem must be set!
    zypper.out().error(_("Specified capability not found"));
    zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
    return false;
  }

  // for many problems, list them shortly first
  //! \todo handle resolver problems caused by --capability mode arguments specially to give proper output (bnc #337007)
  if (rproblems.size() > 1)
  {
    for (i = b; i != e; ++i)
      zypper.out().info(boost::str(
        format(_("Problem: %s")) % (*i)->description()));
  }
  // now list all problems with solution proposals
  for (i = b; i != e; ++i)
  {
    zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL); // visual separator
    tribool stopnow = show_problem(zypper, *(*i), todo);
    if (! indeterminate (stopnow)) {
      retry = stopnow == true;
      break;
    }
  }

  if (retry)
  {
    zypper.out().info(_("Resolving dependencies..."));
    resolver->applySolutions (todo);
  }
  return retry;
}

typedef map<Resolvable::Kind,set<ResObject::constPtr> > KindToResObjectSet;

static void show_summary_resolvable_list(const string & label,
                                         KindToResObjectSet::const_iterator it,
                                         Out & out)
{
  ostringstream s;
  s << endl << label << endl;

  // get terminal width from COLUMNS env. var.
  unsigned cols = 0, cols_written = 0;
  const char *cols_s = getenv("COLUMNS");
  string cols_str("80");
  if (cols_s != NULL)
    cols_str = cols_s;
  str::strtonum (cols_str, cols);
  if (cols == 0)
    cols = 77;

#define INDENT "  "
//! \todo make function to wrap & indent the text 
  for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
      resit != it->second.end(); ++resit)
  {
    ResObject::constPtr res(*resit);

    if (out.verbosity() == Out::NORMAL)
    {
      // watch the terminal widht
      if (cols_written == 0)
        s << INDENT;
      else if (cols_written + res->name().size() + 1  > cols)
      {
        s << endl;
        cols_written = 0;
      }

      cols_written += res->name().size();
    }
    else
      s << INDENT;

    // resolvable name
    s << res->name() << (out.verbosity() > Out::NORMAL ? "" : " ");
    // plus edition and architecture for verbose output
    if (out.verbosity() > Out::NORMAL)
    {
      s << "-" << res->edition() << "." << res->arch();

      const string & reponame =  res->repoInfo().name();
      if (!res->vendor().empty() || !reponame.empty())
      {
        s << "  (";
        // plus repo providing this package
        if (!reponame.empty())
          s << reponame;
        // plus package vendor
        if (!res->vendor().empty())
          s << (reponame.empty() ? "" : ", ") << res->vendor();
        s << ")";
      }
      // new line after each package in the verbose mode
      s << endl;
    }
  }

  if (out.verbosity() == Out::NORMAL)
    s << endl;

  out.info(s.str(), Out::QUIET); //! \todo special output needed for this
}

typedef enum
{
  TO_UPGRADE,
  TO_DOWNGRADE,
  TO_INSTALL,
  TO_REINSTALL,
  TO_REMOVE,
  TO_CHANGE_ARCH,
  TO_CHANGE_VENDOR
} SummaryType;

static void xml_print_to_transact_tag(SummaryType stype, bool end = false)
{
  switch (stype)
  {
  case TO_UPGRADE:
    cout << "<" << (end ? "/" : "") << "to-upgrade>" << endl;
    break;
  case TO_DOWNGRADE:
    cout << "<" << (end ? "/" : "") << "to-downgrade>" << endl;
    break;
  case TO_INSTALL:
    cout << "<" << (end ? "/" : "") << "to-install>" << endl;
    break;
  case TO_REINSTALL:
    cout << "<" << (end ? "/" : "") << "to-reinstall>" << endl;
    break;
  case TO_REMOVE:
    cout << "<" << (end ? "/" : "") << "to-remove>" << endl;
    break;
  case TO_CHANGE_ARCH:
    cout << "<" << (end ? "/" : "") << "to-change-arch>" << endl;
    break;
  case TO_CHANGE_VENDOR:
    cout << "<" << (end ? "/" : "") << "to-change-vendor>" << endl;
    break;
  }
}

static void show_summary_of_type(Zypper & zypper,
                                 SummaryType stype,
                                 const KindToResObjectSet & summset)
{
  // xml install summary
  if (zypper.out().type() == Out::TYPE_XML)
  {
    bool empty = true;
    for (KindToResObjectSet::const_iterator it = summset.begin();
        it != summset.end(); ++it)
      if (!it->second.empty()) { empty = false; break; }
    if (empty)
      return;

    xml_print_to_transact_tag(stype);

    for (KindToResObjectSet::const_iterator it = summset.begin();
        it != summset.end(); ++it)
      for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
          resit != it->second.end(); ++resit)
      {
        ResObject::constPtr res(*resit);

        cout << "<solvable";
        cout << " type=\"" << it->first << "\"";
        cout << " name=\"" << res->name() << "\"";
        cout << " edition=\"" << res->edition() << "\"";
        //! \todo cout << " edition-old=\"" << << "\""; 
        cout << " arch=\"" << res->arch() << "\"";
        if (!res->summary().empty())
          cout << " summary=\"" << xml_encode(res->summary()) << "\"";
        if (!res->description().empty())
          cout << ">" << endl << xml_encode(res->description()) << "</solvable>" << endl;
        else
          cout << "/>" << endl;
      }

    xml_print_to_transact_tag(stype, true);

    return;
  }

  // normal install summary
  for (KindToResObjectSet::const_iterator it = summset.begin();
      it != summset.end(); ++it)
  {
    string title;
    switch (stype)
    {
    case TO_UPGRADE:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to be upgraded:",
          "The following packages are going to be upgraded:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to be upgraded:",
          "The following patches are going to be upgraded:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to be upgraded:",
          "The following patterns are going to be upgraded:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to be upgraded:",
          "The following products are going to be upgraded:",
          it->second.size());
      break;
    case TO_DOWNGRADE:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to be downgraded:",
          "The following packages are going to be downgraded:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to be downgraded:",
          "The following patches are going to be downgraded:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to be downgraded:",
          "The following patterns are going to be downgraded:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to be downgraded:",
          "The following products are going to be downgraded:",
          it->second.size());
      break;
    case TO_INSTALL:
      if (it->first == ResKind::package)
        title = _PL(
          "The following NEW package is going to be installed:",
          "The following NEW packages are going to be installed:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following NEW patch is going to be installed:",
          "The following NEW patches are going to be installed:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following NEW pattern is going to be installed:",
          "The following NEW patterns are going to be installed:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following NEW product is going to be installed:",
          "The following NEW products are going to be installed:",
          it->second.size());
      else if (it->first == ResKind::srcpackage)
        title = _PL(
          "The following source package is going to be installed:",
          "The following source packages are going to be installed:",
          it->second.size());
      break;
    case TO_REINSTALL:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to be reinstalled:",
          "The following packages are going to be reinstalled:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to be reinstalled:",
          "The following patches are going to be reinstalled:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to be reinstalled:",
          "The following patterns are going to be reinstalled:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to be reinstalled:",
          "The following products are going to be reinstalled:",
          it->second.size());
      break;
    case TO_REMOVE:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to be REMOVED:",
          "The following packages are going to be REMOVED:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to be REMOVED:",
          "The following patches are going to be REMOVED:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to be REMOVED:",
          "The following patterns are going to be REMOVED:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to be REMOVED:",
          "The following products are going to be REMOVED:",
          it->second.size());
      break;
    case TO_CHANGE_ARCH:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to change architecture:",
          "The following packages are going to change architecture:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to change architecture:",
          "The following patches are going to change architecture:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to change architecture:",
          "The following patterns are going to change architecture:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to change architecture:",
          "The following products are going to change architecture:",
          it->second.size());
      break;
    case TO_CHANGE_VENDOR:
      if (it->first == ResKind::package)
        title = _PL(
          "The following package is going to change vendor:",
          "The following packages are going to change vendor:",
          it->second.size());
      else if (it->first == ResKind::patch)
        title = _PL(
          "The following patch is going to change vendor:",
          "The following patches are going to change vendor:",
          it->second.size());
      else if (it->first == ResKind::pattern)
        title = _PL(
          "The following pattern is going to change vendor:",
          "The following patterns are going to change vendor:",
          it->second.size());
      else if (it->first == ResKind::product)
        title = _PL(
          "The following product is going to change vendor:",
          "The following products are going to change vendor:",
          it->second.size());
      break;
    }

    show_summary_resolvable_list(title, it, zypper.out());
  }
}

enum
{
  SUMMARY_OK = 0,
  SUMMARY_NOTHING_TO_DO,
  SUMMARY_INSTALL_DOES_REMOVE,
  SUMMARY_REMOVE_DOES_INSTALL
};

/**
 * sets zypper exit code to:
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 * 
 * @return SUMMARY_*
 */
static int summary(Zypper & zypper)
{
  int retv = SUMMARY_NOTHING_TO_DO;

  MIL << "Pool contains " << God->pool().size() << " items." << std::endl;
  DBG << "Install summary:" << endl;


  KindToResObjectSet to_be_installed;
  KindToResObjectSet to_be_removed;

  // collect resolvables to be installed/removed and set the return status
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it )
  {
    ResObject::constPtr res = it->resolvable();
    if ( it->status().isToBeInstalled() || it->status().isToBeUninstalled() )
    {
      if (it->resolvable()->kind() == ResTraits<Patch>::kind)
      {
        Patch::constPtr patch = asKind<Patch>(it->resolvable());

        // set return value to 'reboot needed'
        if (patch->rebootSuggested())
          zypper.setExitCode(ZYPPER_EXIT_INF_REBOOT_NEEDED);
        // set return value to 'restart needed' (restart of package manager)
        // however, 'reboot needed' takes precedence
        else if (zypper.exitCode() != ZYPPER_EXIT_INF_REBOOT_NEEDED && patch->restartSuggested())
          zypper.setExitCode(ZYPPER_EXIT_INF_RESTART_NEEDED);
      }

      if ( it->status().isToBeInstalled()
	   && it->resolvable()->kind() != ResTraits<Atom>::kind )
      {
        DBG << "<install>   ";
        to_be_installed[it->resolvable()->kind()].insert(it->resolvable());
      }
      if ( it->status().isToBeUninstalled()
	   && it->resolvable()->kind() != ResTraits<Atom>::kind )
      {
        DBG << "<uninstall> ";
        to_be_removed[it->resolvable()->kind()].insert(it->resolvable());
      }
      DBG << *res << endl;
    }
  }

  if (!to_be_removed.empty() || !to_be_installed.empty())
    retv = SUMMARY_OK;

  if (retv == SUMMARY_NOTHING_TO_DO && zypper.runtimeData().srcpkgs_to_install.empty())
  {
    if (zypper.command() == ZypperCommand::VERIFY)
      zypper.out().info(_("Dependencies of all installed packages are satisfied."));
    else
      zypper.out().info(_("Nothing to do."));
    return retv;
  }

  if (zypper.command() == ZypperCommand::VERIFY)
    zypper.out().info(_("Some of the dependencies of installed packages are broken."
        " In order to fix these dependencies, the following actions need to be taken:"));

  // total packages to download&install.
  zypper.runtimeData().commit_pkgs_total;
  for (KindToResObjectSet::const_iterator it = to_be_installed.begin();
      it != to_be_installed.end(); ++it)
    zypper.runtimeData().commit_pkgs_total += it->second.size();
  zypper.runtimeData().commit_pkg_current = 0;

  KindToResObjectSet toinstall;
  KindToResObjectSet toupgrade;
  KindToResObjectSet todowngrade;
  KindToResObjectSet toreinstall;
  KindToResObjectSet toremove;
  KindToResObjectSet tochangearch;
  KindToResObjectSet tochangevendor;

  // iterate the to_be_installed to find installs/upgrades/downgrades + size info
  ByteCount download_size, new_installed_size;

  for (KindToResObjectSet::const_iterator it = to_be_installed.begin();
      it != to_be_installed.end(); ++it)
  {
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      ResObject::constPtr res(*resit);

      // find in to_be_removed:
      bool upgrade_downgrade = false;
      for (set<ResObject::constPtr>::iterator rmit = to_be_removed[res->kind()].begin();
          rmit != to_be_removed[res->kind()].end(); ++rmit)
      {
        if (res->name() == (*rmit)->name())
        {
          // upgrade
          if (res->edition() > (*rmit)->edition())
          {
            toupgrade[res->kind()].insert(res);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(res);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(res);
          }
          // reinstall
          else if (res->edition() == (*rmit)->edition())
          {
            toreinstall[res->kind()].insert(res);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(res);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(res);
          }
          // downgrade
          else
          {
            todowngrade[res->kind()].insert(res);
            if (res->arch() != (*rmit)->arch())
              tochangearch[res->kind()].insert(res);
            if (res->vendor() != (*rmit)->vendor())
              tochangevendor[res->kind()].insert(res);
          }

          new_installed_size += res->installsize() - (*rmit)->installsize();

          to_be_removed[res->kind()].erase(*rmit);
          upgrade_downgrade = true;
          break;
        }
      }

      if (!upgrade_downgrade)
      {
        toinstall[res->kind()].insert(res);
        new_installed_size += res->installsize();
      }

      download_size += res->downloadSize();
    }
  }

  bool toremove_by_solver = false;
  for (KindToResObjectSet::const_iterator it = to_be_removed.begin();
      it != to_be_removed.end(); ++it)
    for (set<ResObject::constPtr>::const_iterator resit = it->second.begin();
        resit != it->second.end(); ++resit)
    {
      /** \todo this does not work
      if (!toremove_by_solver)
      {
        PoolItem pi(*resit);
        if (pi.status() == ResStatus::SOLVER)
          toremove_by_solver = true;
      }*/
      toremove[it->first].insert(*resit);
      new_installed_size -= (*resit)->installsize();
    }

  for (list<SrcPackage::constPtr>::const_iterator it = zypper.runtimeData().srcpkgs_to_install.begin();
      it != zypper.runtimeData().srcpkgs_to_install.end(); ++it)
    toinstall[ResTraits<SrcPackage>::kind].insert(*it);

  if (!toremove.empty() && (
      zypper.command() == ZypperCommand::INSTALL ||
      zypper.command() == ZypperCommand::UPDATE))
    retv = SUMMARY_INSTALL_DOES_REMOVE;
  else if ((!toinstall.empty() || toremove_by_solver)
      && zypper.command() == ZypperCommand::REMOVE)
    retv = SUMMARY_REMOVE_DOES_INSTALL;

  // "</install-summary>"
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<install-summary";
    cout << " download-size=\"" << ((ByteCount::SizeType) download_size) << "\"";
    cout << " space-usage-diff=\"" << ((ByteCount::SizeType) new_installed_size) << "\"";
    cout << ">" << endl;
  }

  // show summary
  show_summary_of_type(zypper, TO_UPGRADE, toupgrade);
  show_summary_of_type(zypper, TO_DOWNGRADE, todowngrade);
  show_summary_of_type(zypper, TO_INSTALL, toinstall);
  show_summary_of_type(zypper, TO_REINSTALL, toreinstall);
  show_summary_of_type(zypper, TO_REMOVE, toremove);
  show_summary_of_type(zypper, TO_CHANGE_ARCH, tochangearch);
  show_summary_of_type(zypper, TO_CHANGE_VENDOR, tochangevendor);

  // "</install-summary>"
  if (zypper.out().type() == Out::TYPE_XML)
    cout << "</install-summary>" << endl;

  zypper.out().info("", Out::NORMAL, Out::TYPE_NORMAL); // visual separator

  // count and download size info
  ostringstream s;
  if (download_size > 0)
  {
    s << format(_("Overall download size: %s.")) % download_size;
    s << " ";
  }
  if (new_installed_size > 0)
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    s << format(_("After the operation, additional %s will be used."))
        % new_installed_size.asString(0,1,1);
  else if (new_installed_size == 0)
    s << _("No additional space will be used or freed after the operation.");
  else
  {
    // get the absolute size
    ByteCount abs;
    abs = (-new_installed_size);
    // TrasnlatorExplanation %s will be substituted by a byte count e.g. 212 K
    s << format(_("After the operation, %s will be freed."))
        % abs.asString(0,1,1);
  }
  zypper.out().info(s.str());

  return retv;
}

/*
std::string calculate_token()
{
  SourceManager_Ptr manager;
  manager = SourceManager::sourceManager();

  std::string token;
  stringstream token_stream;
  for ( std::list<Source_Ref>::iterator it = gData.sources.begin(); it !=  gData.sources.end(); ++it )
  {
    Source_Ref src = *it;

//     if ( gSettings.disable_system_sources == SourcesFromSystem )
//     {
//       if ( gSettings.output_type == OutputTypeNice )
//         cout << "Refreshing source " <<  src.alias() << endl;
//       src.refresh();
//     }

    token_stream << "[" << src.alias() << "| " << src.url() << src.timestamp() << "]";
    MIL << "Source: " << src.alias() << " from " << src.timestamp() << std::endl;
  }

  token_stream << "[" << "target" << "| " << God->target()->timestamp() << "]";

  //static std::string digest(const std::string& name, std::istream& is
  token = Digest::digest("sha1", token_stream);

  //if ( gSettings.output_type == OutputTypeSimple )
  //  cout << token;

  MIL << "new token [" << token << "]" << " previous: [" << gSettings.previous_token << "] previous code: " << gSettings.previous_code << std::endl;

  return token;
}
*/

static void dump_pool ()
{
  int count = 1;
  static bool full_pool_shown = false;

  _XDEBUG( "---------------------------------------" );
  for (ResPool::const_iterator it =   God->pool().begin(); it != God->pool().end(); ++it, ++count) {

    if (!full_pool_shown                                    // show item if not shown all before
        || it->status().transacts()                         // or transacts
        || !it->isBroken())                                 // or broken status
    {
      _XDEBUG( count << ": " << *it );
    }
  }
  _XDEBUG( "---------------------------------------" );
  full_pool_shown = true;
}


static void set_force_resolution(Zypper & zypper)
{
  // --force-resolution command line parameter value
  TriBool force_resolution = zypper.runtimeData().force_resolution;

  if (zypper.cOpts().count("force-resolution"))
    force_resolution = true;
  if (zypper.cOpts().count("no-force-resolution"))
  {
    if (force_resolution)
      zypper.out().warning(str::form(
        // translators: meaning --force-resolution and --no-force-resolution
        _("%s conflicts with %s, will use the less aggressive %s"),
          "--force-resolution", "--no-force-resolution", "--no-force-resolution"));
    force_resolution = false;
  }

  // if --force-resolution was not specified on the command line, force
  // the resolution by default for the install and remove commands and the
  // rug_compatible mode. Don't force resolution in non-interactive mode
  // and for update and dist-upgrade command (complex solver request).
  // bnc #369980
  if (indeterminate(force_resolution))
  {
    if (!zypper.globalOpts().non_interactive &&
        (zypper.globalOpts().is_rug_compatible ||
         zypper.command() == ZypperCommand::INSTALL ||
         zypper.command() == ZypperCommand::REMOVE))
      force_resolution = true;
    else
      force_resolution = false;
  }

  // save the setting
  zypper.runtimeData().force_resolution = force_resolution; 

  DBG << "force resolution: " << force_resolution << endl;
  ostringstream s;
  s << _("Force resolution:") << " " << (force_resolution ? _("Yes") : _("No"));
  zypper.out().info(s.str(), Out::HIGH);

  God->resolver()->setForceResolve(force_resolution);
}

static void set_no_recommends(Zypper & zypper)
{
  bool no_recommends = false;
  if (zypper.command() == ZypperCommand::REMOVE)
    // never install recommends when removing packages
    no_recommends = true;
  else
    // install also recommended packages unless --no-recommends is specified
    no_recommends = zypper.cOpts().count("no-recommends");
  DBG << "no recommends (only requires): " << no_recommends << endl;
  God->resolver()->setOnlyRequires(no_recommends);
}


static void set_ignore_recommends_of_installed(Zypper & zypper)
{
  bool ignore = true;
  if (zypper.command() == ZypperCommand::DIST_UPGRADE ||
      zypper.command() == ZypperCommand::INSTALL_NEW_RECOMMENDS)
    ignore = false;
  DBG << "ignore recommends of already installed packages: " << ignore << endl;
  God->resolver()->setIgnoreAlreadyRecommended(ignore);
}


/**
 * Run the solver.
 * 
 * \return <tt>true</tt> if a solution has been found, <tt>false</tt> otherwise 
 */
bool resolve(Zypper & zypper)
{
  dump_pool(); // debug
  set_force_resolution(zypper);
  set_no_recommends(zypper);
  set_ignore_recommends_of_installed(zypper);
  zypper.out().info(_("Resolving dependencies..."), Out::HIGH);
  DBG << "Calling the solver..." << endl;
  return God->resolver()->resolvePool();
}

static bool verify(Zypper & zypper)
{
  dump_pool();
  zypper.out().info(_("Verifying dependencies..."), Out::HIGH);
  // don't force aggressive solutions
  God->resolver()->setForceResolve(false); //! \todo move to set_force_resolution()
  set_no_recommends(zypper);
  DBG << "Calling the solver to verify system..." << endl;
  return God->resolver()->verifySystem();
}

static void make_solver_test_case(Zypper & zypper)
{
  set_force_resolution(zypper);
  set_no_recommends(zypper);

  string testcase_dir("/var/log/zypper.solverTestCase");

  zypper.out().info(_("Generating solver test case..."));
  if (God->resolver()->createSolverTestcase(testcase_dir))
    zypper.out().info(boost::str(
      format(_("Solver test case generated successfully at %s."))
        % testcase_dir));
  else
  {
    zypper.out().error(_("Error creating the solver test case."));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}

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
      if (patch->category() == "security")
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

// returns true if restartSuggested() patches are availble
bool xml_list_patches ()
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
    if (it->isRelevant() && !it->isSatisfied())
    {
      ResObject::constPtr res = it->resolvable();
      Patch::constPtr patch = asKind<Patch>(res);

      // if updates stack patches are available, show only those
      if ((pkg_mgr_available && patch->restartSuggested()) || !pkg_mgr_available)
      {
        cout << " <update ";
        cout << "name=\"" << res->name () << "\" ";
        cout << "edition=\""  << res->edition ().asString() << "\" ";
        cout << "category=\"" <<  patch->category() << "\" ";
        cout << "pkgmanager=\"" << (patch->restartSuggested() ? "true" : "false") << "\" ";
        cout << "restart=\"" << (patch->rebootSuggested() ? "true" : "false") << "\" ";
        cout << "interactive=\"" << (patch->interactive() ? "true" : "false") << "\" ";
        cout << "kind=\"patch\"";
        cout << ">" << endl;
        cout << "  <summary>" << xml_encode(patch->summary()) << "  </summary>" << endl;
        cout << "  <description>" << xml_encode(patch->description()) << "</description>" << endl;
        cout << "  <license>" << xml_encode(patch->licenseToConfirm()) << "</license>" << endl;

        if ( !patch->repoInfo().alias().empty() )
        {
          cout << "  <source url=\"" << xml_encode(patch->repoInfo().baseUrlsBegin()->asString());
          cout << "\" alias=\"" << xml_encode(patch->repoInfo().alias()) << "\"/>" << endl;
        }

        cout << " </update>" << endl;
      }
    }
  }

  if (patchcount == 0)
    cout << "<appletinfo status=\"no-update-repositories\"/>" << endl;

  return pkg_mgr_available;
}

// ----------------------------------------------------------------------------

static void list_patch_updates(Zypper & zypper)
{
  Table tbl;
  Table pm_tbl;	// only those that affect packagemanager (restartSuggested()), they have priority
  TableHeader th;
  unsigned cols;

  th << (zypper.globalOpts().is_rug_compatible ? _("Catalog") : _("Repository"))
     << _("Name") << _("Version") << _("Category") << _("Status");
  cols = 5;
  tbl << th;
  pm_tbl << th;

  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin(ResKind::patch),
    e  = pool.byKindEnd(ResKind::patch);
  for (; it != e; ++it )
  {
    ResObject::constPtr res = it->resolvable();

    if ( it->isRelevant() && ! it->isSatisfied() )
    {
      Patch::constPtr patch = asKind<Patch>(res);

      {
        TableRow tr (cols);
        tr << patch->repoInfo().name();
        tr << res->name () << res->edition ().asString();
        tr << patch->category();
        tr <<  _("Needed");        

        if (patch->restartSuggested ())
          pm_tbl << tr;
        else
          tbl << tr;
      }
    }
  }

  // those that affect the package manager go first
  // (TODO: user option for this?)
  if (!pm_tbl.empty ())
  {
    if (!tbl.empty ())
      zypper.out().warning(
        _("These are only the updates affecting the updater itself.\n"
          "Other updates are available too.\n"));
    tbl = pm_tbl;
  }

  tbl.sort (1); 		// Name

  if (tbl.empty())
    zypper.out().info(_("No updates found."));
  else
    cout << tbl;
}

// ----------------------------------------------------------------------------

// collect items, select best edition
//   this is used to find best available or installed.
// The name of the class is a bit misleading though ...

class LookForArchUpdate : public zypp::resfilter::PoolItemFilterFunctor
{
public:
  PoolItem best;

  bool operator()( PoolItem provider )
    {
      if (!provider.status().isLocked()	// is not locked (taboo)
	  && (!best 			// first match
	      // or a better edition than candidate
	      || best->edition().compare( provider->edition() ) < 0))
      {
	best = provider;	// store
      }
      return true;		// keep going
    }
};

// ----------------------------------------------------------------------------

// Find best (according to edition) uninstalled item
// with same kind/name/arch as item.
// Similar to zypp::solver::detail::Helper::findUpdateItem
// but that allows changing the arch (#222140).
static
PoolItem
findArchUpdateItem( const ResPool & pool, PoolItem item )
{
  LookForArchUpdate info;

  invokeOnEach( pool.byIdentBegin( item->kind(), item->name() ),
                pool.byIdentEnd( item->kind(), item->name() ),
                // get uninstalled, equal kind and arch, better edition
                functor::chain (
                  functor::chain (
                    resfilter::ByUninstalled (),
                    resfilter::byArch<CompareByEQ<Arch> >( item->arch() ) ),
                  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),
                functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findArchUpdateItem(" << item << ") => " << info.best);
  return info.best;
}

// ----------------------------------------------------------------------------

typedef set<PoolItem> Candidates;

/**
 * Find all available updates of given kind.
 */
static void
find_updates( const ResKind & kind, Candidates & candidates )
{
  const zypp::ResPool& pool = God->pool();
  ResPool::byKind_iterator
    it = pool.byKindBegin (kind),
    e  = pool.byKindEnd (kind);
  DBG << "Looking for update candidates of kind " << kind << endl;
  for (; it != e; ++it)
  {
    if (it->status().isUninstalled())
      continue;
    // (actually similar to ProvideProcess?)
    PoolItem candidate = findArchUpdateItem( pool, *it );
    if (!candidate.resolvable())
      continue;

    DBG << "item " << *it << endl;
    DBG << "cand " << candidate << endl;
    candidates.insert (candidate);
  }
}

// ----------------------------------------------------------------------------

/**
 * Find all available updates of given kinds.
 */
static void
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
  if (kind == ResTraits<Package>::kind)
    return _("Package updates");
  else if (kind == ResTraits<Patch>::kind)
    return _("Patches");
  else if (kind == ResTraits<Pattern>::kind)
    return _("Pattern updates");
  else if (kind == ResTraits<Product>::kind)
    return _("Product updates");

  return boost::str(format("%s updates") % kind);
}

// ----------------------------------------------------------------------------

void list_updates(Zypper & zypper, const ResKindSet & kinds, bool best_effort)
{
  if (zypper.out().type() == Out::TYPE_XML)
  {
    cout << "<update-status version=\"0.6\">" << endl;
    cout << "<update-list>" << endl;
  }
  bool not_affects_pkgmgr = false;

  unsigned kind_size = kinds.size();
  ResKindSet localkinds = kinds;
  ResKindSet::iterator it;

  // patch updates
  it = localkinds.find(ResKind::patch);
  if(it != localkinds.end())
  {
    if (zypper.out().type() == Out::TYPE_XML)
      not_affects_pkgmgr = !xml_list_patches();
    else
    {
      zypper.out().info(i18n_kind_updates(*it), Out::QUIET, Out::TYPE_NORMAL);
      zypper.out().info("", Out::QUIET, Out::TYPE_NORMAL); // visual separator
      list_patch_updates(zypper);
    }
    localkinds.erase(it);
  }

  // other kinds
  //! \todo list package updates according to Resolver::doUpdate()
  if (zypper.out().type() == Out::TYPE_XML)
  {
    if (not_affects_pkgmgr)
      xml_list_updates(localkinds);
    cout << "</update-list>" << endl;
    cout << "</update-status>" << endl;
    return;
  }

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
    if (!hide_repo) {
      th << (zypper.globalOpts().is_rug_compatible ? _("Catalog") : _("Repository"));
    }
    if (zypper.globalOpts().is_rug_compatible) {
      th << _("Bundle");
    }
    name_col = th.cols();
    th << _("Name");
    if (!best_effort) {		// best_effort does not know version or arch yet
      th << _("Version") << _("Arch");
    }
    tbl << th;

    unsigned int cols = th.cols();

    Candidates candidates;
    find_updates( *it, candidates );

    Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
    for (ci = cb; ci != ce; ++ci) {
//      ResStatus& candstat = ci->status();
//      candstat.setToBeInstalled (ResStatus::USER);
      ResObject::constPtr res = ci->resolvable();
      TableRow tr (cols);
      tr << "v";
      if (!hide_repo) {
	tr << res->repoInfo().name();
      }
      if (zypper.globalOpts().is_rug_compatible)
	tr << "";		// Bundle
      tr << res->name ();

      // strictly speaking, we could show version and arch even in best_effort
      //  iff there is only one candidate. But we don't know the number of candidates here.
      if (!best_effort) {
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

// may be useful as a functor
static bool
mark_item_install (const PoolItem & pi)
{
  bool result = pi.status().setToBeInstalled( zypp::ResStatus::USER );
  if (!result)
    ERR << "Marking " << pi << "for installation failed" << endl;
  return result;
}

// ----------------------------------------------------------------------------
// best-effort update
// ----------------------------------------------------------------------------

// find installed item matching passed one
//   use LookForArchUpdate as callback handler in order to cope with
//   multiple installed resolvables of the same name.
//   LookForArchUpdate will return the one with the highest edition.

static PoolItem
findInstalledItem( PoolItem item )
{
  const zypp::ResPool& pool = God->pool();
  LookForArchUpdate info;

  invokeOnEach( pool.byIdentBegin( item->kind(), item->name() ),
                pool.byIdentEnd( item->kind(), item->name() ),
                resfilter::ByInstalled (),
                functor::functorRef<bool,PoolItem> (info) );

  _XDEBUG("findInstalledItem(" << item << ") => " << info.best);
  return info.best;
}

// ----------------------------------------------------------------------------

// require update of installed item
//   The PoolItem passed to require_item_update() is the installed resolvable
//   to which an update candidate is guaranteed to exist.
//
// may be useful as a functor

static bool require_item_update (const PoolItem& pi) {
  Resolver_Ptr resolver = zypp::getZYpp()->resolver();

  PoolItem installed = findInstalledItem( pi );

  // require anything greater than the installed version
  try {
    Capability  cap( installed->name(), Rel::GT, installed->edition(), installed->kind() );
    resolver->addRequire( cap );
  }
  catch (const Exception& e) {
    ZYPP_CAUGHT(e);
    Zypper::instance()->out().error(boost::str(format(
      _("Cannot parse '%s < %s'")) % installed->name() % installed->edition()));
  }

  return true;
}

// ----------------------------------------------------------------------------

void xml_list_updates(const ResKindSet & kinds)
{
  Candidates candidates;
  find_updates (kinds, candidates);

  Candidates::iterator cb = candidates.begin (), ce = candidates.end (), ci;
  for (ci = cb; ci != ce; ++ci) {
    ResObject::constPtr res = ci->resolvable();

    cout << " <update ";
    cout << "name=\"" << res->name () << "\" " ;
    cout << "edition=\""  << res->edition ().asString() << "\" ";
    cout << "kind=\"" << res->kind() << "\" ";
    cout << ">" << endl;
    cout << "  <summary>" << xml_encode(res->summary()) << "  </summary>" << endl;
    cout << "  <description>" << xml_encode(res->description()) << "</description>" << endl;
    cout << "  <license>" << xml_encode(res->licenseToConfirm()) << "</license>" << endl;

    if ( !res->repoInfo().alias().empty() )
    {
    	cout << "  <source url=\"" << xml_encode(res->repoInfo().baseUrlsBegin()->asString());
    	cout << "\" alias=\"" << xml_encode(res->repoInfo().alias()) << "\"/>" << endl;
    }

    cout << " </update>" << endl;
  }
}

// ----------------------------------------------------------------------------

static bool
mark_patch_update(const PoolItem & pi, bool skip_interactive, bool ignore_affects_pm)
{
  Patch::constPtr patch = asKind<Patch>(pi.resolvable());
  if (pi.isRelevant() && !pi.isSatisfied())
  {
    DBG << "patch " << patch->name() << " " << ignore_affects_pm << ", "
      << patch->restartSuggested() << endl;
    if (ignore_affects_pm || patch->restartSuggested())
    {
      // #221476
      if (skip_interactive
          && (patch->interactive() || !patch->licenseToConfirm().empty()))
      {
        // Skipping a patch because it is marked as interactive or has
        // license to confirm and --skip-interactive is requested.
        Zypper::instance()->out().warning(str::form(
          // translators: %s is the name of a patch
          _("'%s' is interactive, skipping."), patch->name().c_str()));
        return false;
      }
      else
      {
        mark_item_install(pi);
        return true;
      }
    }
  }

  return false;
}

// ----------------------------------------------------------------------------

static void
mark_patch_updates( Zypper & zypper, bool skip_interactive )
{
  DBG << "going to mark patches to install" << endl;

  // search twice: if there are none with restartSuggested(), retry on all
  bool any_marked = false;
  for(unsigned ignore_affects_pm = 0;
      !any_marked && ignore_affects_pm < 2; ++ignore_affects_pm)
  {
    if (zypper.arguments().empty() || zypper.globalOpts().is_rug_compatible)
    {
      DBG << "marking all needed patches" << endl;

      for_(it, God->pool().byKindBegin(ResKind::patch), 
               God->pool().byKindEnd  (ResKind::patch))
      {
        if (mark_patch_update(*it, skip_interactive, ignore_affects_pm))
          any_marked = true;
      }
    }
    else if (!zypper.arguments().empty())
    {
      for_(it, zypper.arguments().begin(), zypper.arguments().end())
      {
        // look for patches matching specified pattern
        PoolQuery q;
        q.addKind(ResKind::patch);
        q.addAttribute(sat::SolvAttr::name, *it);
        //! \todo should we look for patches requiring packages with matching name instead? 
        //q.addAttribute(sat::SolvAttr::require, *it);
        q.setMatchGlob();

        if (q.empty())
        {
          if (ignore_affects_pm) // avoid displaying this twice
            continue;
          if (it->find_first_of("?*") != string::npos) // wildcards used
            zypper.out().info(str::form(
                _("No patches matching '%s' found."), it->c_str()));
          else
            zypper.out().info(str::form(
                _("Patch '%s' not found."), it->c_str()));
          zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        }
        else
        {
          for_(pit, q.poolItemBegin(), q.poolItemEnd())
          {
            any_marked = mark_patch_update(*pit, skip_interactive, ignore_affects_pm);
          }
        }
      }
    }
  }
}

// ----------------------------------------------------------------------------

void mark_updates(Zypper & zypper, const ResKindSet & kinds, bool skip_interactive, bool best_effort )
{
  ResKindSet localkinds = kinds;

  if (zypper.arguments().empty() || zypper.globalOpts().is_rug_compatible)
  {
    for_(kindit, localkinds.begin(), localkinds.end())
    {
      if (*kindit == ResKind::package)
      {
        // this will do a complete pacakge update as far as possible
        God->resolver()->doUpdate();
        // no need to call Resolver::resolvePool() afterwards
        zypper.runtimeData().solve_before_commit = false;
      }
      else if (*kindit == ResKind::patch)
      {
        mark_patch_updates(zypper, skip_interactive);
      }
      else
      {
        Candidates candidates;
        find_updates (localkinds, candidates);
        if (best_effort)
          invokeOnEach (candidates.begin(), candidates.end(), require_item_update);
        else
          invokeOnEach (candidates.begin(), candidates.end(), mark_item_install);
      } 
    }
  }
  // treat arguments as package names (+allow wildcards)
  else if (!zypper.arguments().empty())
  {
    for_(kindit, localkinds.begin(), localkinds.end())
    {
      Resolver_Ptr solver = God->resolver();
      for_(it, zypper.arguments().begin(), zypper.arguments().end())
      {
        PoolQuery q;
        q.addKind(*kindit);
        q.addAttribute(sat::SolvAttr::name, *it);
        q.setMatchGlob();
        q.setInstalledOnly();
  
        if (q.empty())
        {
          if (it->find_first_of("?*") != string::npos) // wildcards used
            zypper.out().info(str::form(
                _("No packages matching '%s' are installed."), it->c_str()));
          else
            zypper.out().info(str::form(
                _("Package '%s' is not installed."), it->c_str()));
          zypper.setExitCode(ZYPPER_EXIT_INF_CAP_NOT_FOUND);
        }
        else
          for_(solvit, q.selectableBegin(), q.selectableEnd())
          {
            ui::Selectable::Ptr s = *solvit;
            PoolItem theone = s->theObj();
            if (equalNVRA(*s->installedObj().resolvable(), *theone.resolvable()))
            {
              DBG << "the One (" << theone << ") is installed, skipping." << endl;
              zypper.out().info(str::form(
                  _("No update candidate for '%s'."), s->name().c_str()));
            }
            else
            {
              //s->setCandidate(theone); ?
              //s->setStatus(ui::S_Update); ?
              Capability c(s->name(), Rel::GT, s->installedObj()->edition(), s->kind());
              solver->addRequire(c);
              DBG << *s << " update: adding requirement " << c << endl;
            }
          }
      }
    }
  }
}

// ----------------------------------------------------------------------------
// commit
// ----------------------------------------------------------------------------

/**
 * @return ZYPPER_EXIT_OK - successful commit,
 *  ZYPPER_EXIT_ERR_ZYPP - if ZYppCommitResult contains resolvables with errors,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */
void solve_and_commit (Zypper & zypper)
{
  if (zypper.cOpts().count("debug-solver"))
  {
    make_solver_test_case(zypper);
    return;
  }

  bool show_forced_problems = true;
  bool commit_done = false;
  do
  {
    if (zypper.runtimeData().solve_before_commit) // doUpdate was called, no need for solving
    {
      MIL << "solving..." << endl;

      while (true)
      {
        bool success;
        if (zypper.command() == ZypperCommand::VERIFY)
          success = verify(zypper);
        else if (zypper.command() == ZypperCommand::DIST_UPGRADE)
        {
          zypp::UpgradeStatistics opt_stats;
          //! \todo set success to doUpgrade return value if there are problems
          success = false;
          God->resolver()->doUpgrade(opt_stats);
          //! \todo remove this hack once the doUpgrade returns bool
          if (God->resolver()->problems().empty())
            break;
        }
        else
          success = resolve(zypper);
        if (success)
          break;

        success = show_problems(zypper);
        if (! success) {
          // TODO cancel transaction?
          zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP); // #242736
          return;
        }
      }
    }

    MIL << "got solution, showing summary" << endl;

    // returns SUMMARY_*
    int retv = summary(zypper);
    if (retv != SUMMARY_NOTHING_TO_DO || !zypper.runtimeData().srcpkgs_to_install.empty())
    {
      // check root user
      if (zypper.command() == ZypperCommand::VERIFY && geteuid() != 0
        && !zypper.globalOpts().changedRoot)
      {
        zypper.out().error(
          _("Root privileges are required to fix broken package dependencies."));
        zypper.setExitCode(ZYPPER_EXIT_ERR_PRIVILEGES);
        return;
      }
  
      bool do_commit = false;
      if (zypper.runtimeData().force_resolution &&
          (retv == SUMMARY_INSTALL_DOES_REMOVE || retv == SUMMARY_REMOVE_DOES_INSTALL))
      {
        PromptOptions popts;
        // translators: Yes / No / show Problems. This prompt will appear
        // after install/update command summary if there will be any package
        // to-be-removed automatically to show why, if asked.
        popts.setOptions(_("y/n/p"), 0);
        // translators: help text for 'y' option in the y/n/p prompt
        popts.setOptionHelp(0, _("Accept the summary and proceed with installation/removal of packages."));
        // translators: help text for 'n' option in the y/n/p prompt
        popts.setOptionHelp(1, _("Cancel the operation."));
        // translators: help text for 'p' option in the y/n/p prompt
        popts.setOptionHelp(2, _("Restart solver in no-force-resolution mode in order to show dependency problems."));
        string prompt_text = _("Continue?");
        zypper.out().prompt(PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts);
        unsigned int reply =
          get_prompt_reply(zypper, PROMPT_YN_INST_REMOVE_CONTINUE, popts);

        if (reply == 2)
        {
          // one more solver solver run with force-resoltion off
          zypper.runtimeData().force_resolution = false;
          // undo solver changes before retrying
          God->resolver()->undo();
          continue;
        }
        else if (reply == 1)
          show_forced_problems = false;
        else
        {
          do_commit = true;
          show_forced_problems = false;
        }
      }
      // no dependency problems
      else
      {
        do_commit = read_bool_answer(PROMPT_YN_INST_REMOVE_CONTINUE, _("Continue?"), true);
        show_forced_problems = false;
      }

      if (do_commit)
      {
        if (!confirm_licenses(zypper))
          return;

        if (retv >= 0)
        {
          try
          {
            gData.show_media_progress_hack = true;
    
            ostringstream s;
            s << _("committing"); MIL << "committing...";
    
            ZYppCommitResult result;
            if (copts.count("dry-run"))
            {
              s << " " << _("(dry run)") << endl; MIL << "(dry run)";
              zypper.out().info(s.str(), Out::HIGH);
    
              result = God->commit(ZYppCommitPolicy().dryRun(true));
            }
            else
            {
              zypper.out().info(s.str(), Out::HIGH);
    
              result = God->commit(
                ZYppCommitPolicy().syncPoolAfterCommit(zypper.runningShell()));
    
              commit_done = true;
            }


            MIL << endl << "DONE" << endl;

            gData.show_media_progress_hack = false;

            if (!result._errors.empty())
              retv = ZYPPER_EXIT_ERR_ZYPP;
    
            s.clear(); s << result;
            zypper.out().info(s.str(), Out::HIGH);
          }
          catch ( const media::MediaException & e ) {
            ZYPP_CAUGHT(e);
            zypper.out().error(e,
                _("Problem downloading the package file from the repository:"),
                _("Please see the above error message for a hint."));
            zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
            return;
          }
          catch ( zypp::repo::RepoException & e ) {
            ZYPP_CAUGHT(e);
            
            RepoManager manager(zypper.globalOpts().rm_options );
  
            bool refresh_needed = false;
            for(RepoInfo::urls_const_iterator it = e.info().baseUrlsBegin();
                      it != e.info().baseUrlsEnd(); ++it)
              {
                RepoManager::RefreshCheckStatus stat = manager.
                              checkIfToRefreshMetadata(e.info(), *it,
                              RepoManager::RefreshForced );
                if ( stat == RepoManager::REFRESH_NEEDED )
                {
                  refresh_needed = true;
                  break;
                }
              }
            
            std::string hint = _("Please see the above error message for a hint.");
            if (refresh_needed)
            {
              hint = boost::str(format(
                  // translators: the first %s is 'zypper refresh' and the second
                  // is repo allias
                  _("Repository '%s' is out of date. Running '%s' might help.")) %
                  e.info().alias() % "zypper refresh" );
            }
            zypper.out().error(e,
                _("Problem downloading the package file from the repository:"),
                hint);
            zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
            return;
          }
          catch ( const zypp::FileCheckException & e ) {
            ZYPP_CAUGHT(e);
            zypper.out().error(e,
                _("The package integrity check failed. This may be a problem"
                " with the repository or media. Try one of the following:\n"
                "\n"
                "- just retry previous command\n"
                "- refresh the repositories using 'zypper refresh'\n"
                "- use another installation medium (if e.g. damaged)\n"
                "- use another repository"));
            zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
            return;
          }
          catch ( const Exception & e ) {
            ZYPP_CAUGHT(e);
            zypper.out().error(e,
                _("Problem occured during or after installation or removal of packages:"),
                _("Please see the above error message for a hint."));
            zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
          }
        }
        // install any pending source packages
        if (!zypper.runtimeData().srcpkgs_to_install.empty())
          install_src_pkgs(zypper);
      }
    }
    // noting to do
    else
      break;
  }
  while (show_forced_problems);

  if (commit_done)
  {
    if (zypper.exitCode() == ZYPPER_EXIT_INF_REBOOT_NEEDED)
      zypper.out().warning(
        _("One of installed patches requires reboot of"
          " your machine. Reboot as soon as possible."), Out::QUIET);
    else if (zypper.exitCode() == ZYPPER_EXIT_INF_RESTART_NEEDED)
      zypper.out().warning(
        _("One of installed patches affects the package"
          " manager itself, thus it requires its restart before executing"
          " any further operations."),
        Out::QUIET, Out::TYPE_NORMAL); // don't show this to machines
  }
}

// TODO confirm licenses
// - make this more user-friendly e.g. show only license name and
//  ask for [y/n/r] with 'r' for read the license text
//  (opened throu more or less, etc...)
// - after negative answer, call solve_and_commit() again
static bool confirm_licenses(Zypper & zypper)
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
