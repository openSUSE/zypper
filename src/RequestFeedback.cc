/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file RequestFeedback.cc
 * 
 */

#include "zypp/base/LogTools.h"
#include "zypp/ui/Selectable.h"

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
  sat::Solvable::SplitIdent splid(_reqcap.detail().name());
  switch (_id)
  {
  case NOT_FOUND_NAME_TRYING_CAPS:
    return str::form(
        _("'%s' not found in package names. Trying capabilities."),
        _reqcap.asString().c_str());

  case NOT_FOUND_NAME:
    if (_reqrepo.empty() && opts.from_repos.empty())
    {
      if (splid.kind() == ResKind::package)
        return str::form(_("Package '%s' not found."), splid.name().c_str());
      else if (splid.kind() == ResKind::patch)
        return str::form(_("Patch '%s' not found."), splid.name().c_str());
      else if (splid.kind() == ResKind::product)
        return str::form(_("Product '%s' not found."), splid.name().c_str());
      else if (splid.kind() == ResKind::pattern)
        return str::form(_("Pattern '%s' not found."), splid.name().c_str());
      else if (splid.kind() == ResKind::srcpackage)
        return str::form(_("Source package '%s' not found."), splid.name().c_str());
      else // just in case
        return str::form(_("Object '%s' not found."), splid.name().c_str());
    }
    else
    {
      if (splid.kind() == ResKind::package)
        return str::form(_("Package '%s' not found in specified repositories."), splid.name().c_str());
      else if (splid.kind() == ResKind::patch)
        return str::form(_("Patch '%s' not found in specified repositories."), splid.name().c_str());
      else if (splid.kind() == ResKind::product)
        return str::form(_("Product '%s' not found in specified repositories."), splid.name().c_str());
      else if (splid.kind() == ResKind::pattern)
        return str::form(_("Pattern '%s' not found in specified repositories."), splid.name().c_str());
      else if (splid.kind() == ResKind::srcpackage)
        return str::form(_("Source package '%s' not found in specified repositories."), splid.name().c_str());
      else // just in case
        return str::form(_("Object '%s' not found in specified repositories."), splid.name().c_str());
    }
  case NOT_FOUND_CAP:
    // translators: meaning a package %s or provider of capability %s
    return str::form(_("No provider of '%s' found."), _reqcap.asString().c_str());

  case NOT_INSTALLED:
    if (splid.name().asString().find_first_of("?*") != string::npos) // wildcards used
      return str::form(
        _("No package matching '%s' are installed."), _reqcap.asString().c_str());
    else
      return str::form(
        _("Package '%s' is not installed."), _reqcap.asString().c_str());

  case NO_INSTALLED_PROVIDER:
    // translators: meaning provider of capability %s
    return str::form(_("No provider of '%s' is installed."), _reqcap.asString().c_str());

  case ALREADY_INSTALLED:
    // TODO Package/Pattern/Patch/Product
    if (_objinst->name() == splid.name())
      return str::form(
          _("'%s' is already installed."), _reqcap.asString().c_str());
    else
      return str::form(
          // translators: %s are package names
          _("'%s' providing '%s' is already installed."),
          _objinst->name().c_str(), _reqcap.asString().c_str());

  case NO_UPD_CANDIDATE:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    if (identical(_objinst, highest) || _objinst->edition() > highest->edition())
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
          " specified version, architecture, or repository."),
        poolitem_user_string(highest).c_str(),
        poolitem_user_string(_objinst).c_str());
  }

  case UPD_CANDIDATE_CHANGES_VENDOR:
  {
    PoolItem highest = asSelectable()(_objinst)->highestAvailableVersionObj();
    ostringstream cmdhint;
    cmdhint << "zypper install " << poolitem_user_string(highest);

    return str::form(
      _("There is an update candidate for '%s', but it is from different"
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
      _("There is an update candidate for '%s', but it comes from repository"
         " with lower priority. Use '%s' to install this candidate."),
        _objinst->name().c_str(), cmdhint.str().c_str());
  }

  case SET_TO_INSTALL:
    if (opts.force)
      return str::form(
          _("Forcing installation of '%s' from repository '%s'."),
          resolvable_user_string(*_objsel.resolvable()).c_str(),
          Zypper::instance()->config().show_alias ?
              _objsel->repoInfo().alias().c_str() :
              _objsel->repoInfo().name().c_str());
    else
      return str::form(
          _("Selecting '%s' from repository '%s' for installation."),
          resolvable_user_string(*_objsel.resolvable()).c_str(),
          Zypper::instance()->config().show_alias ?
              _objsel->repoInfo().alias().c_str() :
              _objsel->repoInfo().name().c_str());

  case SET_TO_REMOVE:
    return str::form(_("Selecting '%s' for removal."),
        resolvable_user_string(*_objsel.resolvable()).c_str());

  case ADDED_REQUIREMENT:
    return str::form(_("Adding requirement: '%s'."), _reqcap.asString().c_str());

  case ADDED_CONFLICT:
    return str::form(_("Adding conflict: '%s'."), _reqcap.asString().c_str());

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
    out.info(asUserString(opts));
    break;
  case SET_TO_INSTALL:
  case SET_TO_REMOVE:
  case ADDED_REQUIREMENT:
  case ADDED_CONFLICT:
    out.info(asUserString(opts), Out::HIGH);
    break;
  default:
    INT << "unknown feedback id? " << _id << endl;
  }
}
