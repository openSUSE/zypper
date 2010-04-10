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

#include "Zypper.h"
#include "SolverRequester.h"

using namespace std;
using namespace zypp;

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
    if (_objinst->name() == splid.name())
      return str::form(
          _("'%s' is already installed."), _reqcap.asString().c_str());
    else
      return str::form(
          // translators: %s are package names
          _("'%s' providing '%s' is already installed."),
          _objinst->name().c_str(), _reqcap.asString().c_str());

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
    out.info(asUserString(opts));
    break;
  default:
    INT << "unknown feedback id? " << _id << endl;
  }
}
