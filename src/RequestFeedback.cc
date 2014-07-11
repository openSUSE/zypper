/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file RequestFeedback.cc
 *
 */

#include <zypp/base/LogTools.h>
#include <zypp/ui/Selectable.h>

#include "Zypper.h"
#include "misc.h"
#include "SolverRequester.h"

using namespace std;
using namespace zypp;
using namespace zypp::ui;


/////////////////////////////////////////////////////////////////////////
// SolverRequester::Feedback
/////////////////////////////////////////////////////////////////////////

string SolverRequester::Feedback::asUserString(
    const SolverRequester::Options & opts) const
{
  sat::Solvable::SplitIdent splid(_reqpkg.parsed_cap.detail().name());
  switch (_id)
  {
  case NOT_FOUND_NAME_TRYING_CAPS:
    return str::form(
        _("'%s' not found in package names. Trying capabilities."),
        _reqpkg.orig_str.c_str());

  case NOT_FOUND_NAME:
    if (_reqpkg.repo_alias.empty() && opts.from_repos.empty())
    {
      if (splid.kind() == ResKind::package)
        return str::form(_("Package '%s' not found."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::patch)
        return str::form(_("Patch '%s' not found."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::product)
        return str::form(_("Product '%s' not found."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::pattern)
        return str::form(_("Pattern '%s' not found."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::srcpackage)
        return str::form(_("Source package '%s' not found."), _reqpkg.orig_str.c_str());
      else // just in case
        return str::form(_("Object '%s' not found."), _reqpkg.orig_str.c_str());
    }
    else
    {
      if (splid.kind() == ResKind::package)
        return str::form(_("Package '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::patch)
        return str::form(_("Patch '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::product)
        return str::form(_("Product '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::pattern)
        return str::form(_("Pattern '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
      else if (splid.kind() == ResKind::srcpackage)
        return str::form(_("Source package '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
      else // just in case
        return str::form(_("Object '%s' not found in specified repositories."), _reqpkg.orig_str.c_str());
    }
  case NOT_FOUND_CAP:
  {
    // translators: meaning a package %s or provider of capability %s
    std::string ret( str::form(_("No provider of '%s' found."), _reqpkg.orig_str.c_str()) );
    if ( _reqpkg.orig_str.find("debuginfo") != std::string::npos )
      ret += " ['--plus-content debug'?]";
    return ret;
  }

  case NOT_INSTALLED:
    if (_reqpkg.orig_str.find_first_of("?*") != string::npos) // wildcards used
      return str::form(
        _("No package matching '%s' is installed."), _reqpkg.orig_str.c_str());
    else
      return str::form(
        _("Package '%s' is not installed."), _reqpkg.orig_str.c_str());

  case NO_INSTALLED_PROVIDER:
    // translators: meaning provider of capability %s
    return str::form(_("No provider of '%s' is installed."), _reqpkg.orig_str.c_str());

  case ALREADY_INSTALLED:
    // TODO Package/Pattern/Patch/Product
    if (_objinst->name() == splid.name())
      return str::form(
          _("'%s' is already installed."), _reqpkg.parsed_cap.asString().c_str());
    else
      return str::form(
          // translators: %s are package names
          _("'%s' providing '%s' is already installed."),
          _objinst->name().c_str(), _reqpkg.parsed_cap.asString().c_str());

  case NO_UPD_CANDIDATE:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    if (highest  && (identical(_objinst, highest) || _objinst->edition() > highest->edition()))
      return str::form(
          _("No update candidate for '%s'."
            " The highest available version is already installed."),
          poolitem_user_string(_objinst).c_str());
    else
      return
        str::form(_("No update candidate for '%s'."), _objinst->name().c_str());
  }

  case UPD_CANDIDATE_USER_RESTRICTED:
  {
    PoolItem highest = asSelectable()(_objsel)->highestAvailableVersionObj();
    return str::form(
        _("There is an update candidate '%s' for '%s', but it does not match"
          " the specified version, architecture, or repository."),
        poolitem_user_string(highest).c_str(),
        poolitem_user_string(_objinst).c_str());
  }

  case UPD_CANDIDATE_CHANGES_VENDOR:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    ostringstream cmdhint;
    cmdhint << "zypper install " << poolitem_user_string(highest);

    return str::form(
      _("There is an update candidate for '%s', but it is from a different"
        " vendor. Use '%s' to install this candidate."),
        _objinst->name().c_str(), cmdhint.str().c_str());
  }

  case UPD_CANDIDATE_HAS_LOWER_PRIO:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    ostringstream cmdhint;
    cmdhint << "zypper install " << highest->name()
        << "-" << highest->edition() << "." << highest->arch();

    return str::form(
      _("There is an update candidate for '%s', but it comes from a repository"
         " with a lower priority. Use '%s' to install this candidate."),
        _objinst->name().c_str(), cmdhint.str().c_str());
  }

  case UPD_CANDIDATE_IS_LOCKED:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    ostringstream cmdhint;
    cmdhint << "zypper removelock " << highest->name();

    return str::form(
        _("There is an update candidate for '%s', but it is locked."
          " Use '%s' to unlock it."),
        _objinst->name().c_str(), cmdhint.str().c_str());
  }

  case NOT_IN_REPOS:
  {
    return str::form(
        _("Package '%s' is not available in your repositories."
          " Cannot reinstall, upgrade, or downgrade."),
        _objinst->name().c_str());
  }

  case SELECTED_IS_OLDER:
  {
    ostringstream cmdhint;
    cmdhint << "zypper install --oldpackage " << _objsel->name() << "-" << _objsel->edition()
        << "." << _objsel->arch();

    ostringstream msg;
    msg << str::form(_(
      "The selected package '%s' from repository '%s' has lower"
      " version than the installed one."),
      resolvable_user_string(*_objsel.resolvable()).c_str(),
      _objsel->repoInfo().asUserString().c_str() );
    msg << " ";
    msg << str::form(
        // translators: %s = "zypper install --oldpackage package-version.arch"
        _("Use '%s' to force installation of the package."), cmdhint.str().c_str());
    return msg.str();
  }

  case PATCH_INTERACTIVE_SKIPPED:
  {
    ostringstream pname;
    pname << _objsel->name() << "-" << _objsel->edition();
    return str::form(
        _("Patch '%s' is interactive, skipping."), pname.str().c_str());
  }

  case PATCH_NOT_NEEDED:
  {
    ostringstream pname;
    pname << _objsel->name() << "-" << _objsel->edition();
    return str::form(_("Patch '%s' is not needed."), pname.str().c_str());
  }

  case PATCH_UNWANTED:
  {
    ostringstream pname;
    pname << _objsel->name() << "-" << _objsel->edition();
    string cmd1 = "zypper in --force patch:" + _objsel->name();
    string cmd2 = "zypper rl patch:" + _objsel->name();
    return str::form(_("Patch '%s' is locked. Use '%s' to install it, or unlock it using '%s'."),
        pname.str().c_str(), cmd1.c_str(), cmd2.c_str());
  }

  case PATCH_WRONG_CAT:
  {
    ostringstream pname;
    pname << _objsel->name() << "-" << _objsel->edition();
    return str::form(_("Patch '%s' is not in the specified category."),
	pname.str().c_str());
  }

  case PATCH_TOO_NEW:
  {
    ostringstream pname;
    pname << _objsel->name() << "-" << _objsel->edition();
    return str::form(_("Patch '%s' was issued after the specified date."),
        pname.str().c_str());
  }

  case SET_TO_INSTALL:
    return str::form(
        _("Selecting '%s' from repository '%s' for installation."),
        resolvable_user_string(*_objsel.resolvable()).c_str(),
        _objsel->repoInfo().asUserString().c_str() );

  case FORCED_INSTALL:
    return str::form(
        _("Forcing installation of '%s' from repository '%s'."),
        resolvable_user_string(*_objsel.resolvable()).c_str(),
        _objsel->repoInfo().asUserString().c_str() );

  case SET_TO_REMOVE:
    return str::form(_("Selecting '%s' for removal."),
        resolvable_user_string(*_objsel.resolvable()).c_str());

  case INSTALLED_LOCKED:
  {
    ostringstream cmdhint;
    cmdhint << "zypper removelock " << _objsel->name();
    return str::form(
        _("'%s' is locked. Use '%s' to unlock it."),
        _objsel->name().c_str(), cmdhint.str().c_str());
  }

  case ADDED_REQUIREMENT:
    return str::form(_("Adding requirement: '%s'."), _reqpkg.parsed_cap.asString().c_str());

  case ADDED_CONFLICT:
    return str::form(_("Adding conflict: '%s'."), _reqpkg.parsed_cap.asString().c_str());

  default:
    INT << "unknown feedback id? " << _id << endl;
    return "You should not see this message. Please report this bug.";
  }

  return string();
}

void SolverRequester::Feedback::print(
    Out & out, const SolverRequester::Options & opts) const
{
  switch (_id)
  {
  case NOT_FOUND_NAME:
  case NOT_FOUND_CAP:
    out.error(asUserString(opts));
    break;
  case NOT_FOUND_NAME_TRYING_CAPS:
  case NOT_INSTALLED:
  case NO_INSTALLED_PROVIDER:
  case ALREADY_INSTALLED:
  case NO_UPD_CANDIDATE:
  case UPD_CANDIDATE_USER_RESTRICTED:
  case UPD_CANDIDATE_CHANGES_VENDOR:
  case UPD_CANDIDATE_HAS_LOWER_PRIO:
  case UPD_CANDIDATE_IS_LOCKED:
  case NOT_IN_REPOS:
  case SELECTED_IS_OLDER:
  case PATCH_NOT_NEEDED:
  case PATCH_UNWANTED:
  case PATCH_WRONG_CAT:
  case PATCH_TOO_NEW:
  case FORCED_INSTALL:
    out.info(asUserString(opts));
    break;
  case SET_TO_INSTALL:
  case SET_TO_REMOVE:
  case ADDED_REQUIREMENT:
  case ADDED_CONFLICT:
    out.info(asUserString(opts), Out::HIGH);
    break;
  case INSTALLED_LOCKED:
    out.warning(asUserString(opts), Out::HIGH);
    break;
  case PATCH_INTERACTIVE_SKIPPED:
    out.warning(asUserString(opts));
    break;
  default:
    INT << "unknown feedback id? " << _id << endl;
  }
}
