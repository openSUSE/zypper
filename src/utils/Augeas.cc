/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include <iostream>
#include <stdlib.h>

#include "zypp/base/Logger.h"
#include "Zypper.h"
#include "utils/Augeas.h"

using namespace zypp;
using namespace std;

Augeas::Augeas()
  : _augeas(NULL), _got_global_zypper_conf(false), _got_user_zypper_conf(false)
{
  MIL << "Going to read zypper config using Augeas..." << endl;

  //! \todo use specified root dir
  _augeas = ::aug_init(NULL, "/usr/share/zypper", AUG_NO_STDINC);

  if (_augeas == NULL)
    ZYPP_THROW(Exception(_("Cannot initialize configuration file parser.")));


  _got_global_zypper_conf =
    ::aug_get(_augeas, "/files/etc/zypp/zypper.conf", NULL) != 0;

  const char *value[1] = {};
  string error;
  if (::aug_get(_augeas, "/augeas/files/etc/zypp/zypper.conf/error/message", value))
    error = value[0];

  _homedir = ::getenv("HOME");
  if (_homedir.empty())
    WAR << "Cannot figure out user's home directory." << endl;
  else
  {
    //! \todo cherry-pick this file as soon as the API allows it
    string user_zypper_conf = "/files" + _homedir + "/.zypper.conf";
    _got_user_zypper_conf =
      ::aug_get(_augeas, user_zypper_conf.c_str(), NULL) != 0;
  }

  if (!_got_global_zypper_conf && !_got_user_zypper_conf)
  {
    if (error.empty())
      ZYPP_THROW(Exception(
          _("No configuration file exists or could be parsed.")));
    else
    {
      string msg = _("Error parsing zypper.conf:") + string("\n") + error;
      ZYPP_THROW(Exception(msg));
    }
  }

  MIL << "Done reading conf files:" << endl;
  MIL << "user conf read: " << (_got_user_zypper_conf ? "yes" : "no") << endl;
  MIL << "global conf read: " << (_got_global_zypper_conf ? "yes" : "no") << endl;
}

// ---------------------------------------------------------------------------

Augeas::~Augeas()
{
  if (_augeas != NULL)
    ::aug_close(_augeas);
}

// ---------------------------------------------------------------------------

static string global_option_path(
    const string & section, const string & option)
{
  return "/files/etc/zypp/zypper.conf/" + section + "/*/" + option;
}

static string user_option_path(
    const string & section, const string & option, const string & homedir)
{
  return "/files" + homedir + "/.zypper.conf/" + section + "/*/" + option;
}

// ---------------------------------------------------------------------------

string Augeas::get(const string & augpath) const
{
  const char *value[1] = {};
  _last_get_result = ::aug_get(_augeas, augpath.c_str(), value);
  if (_last_get_result)
  {
    DBG << "Got " << augpath << " = " << value[0] << endl;
    return value[0];
  }
  else if (_last_get_result == 0)
    DBG << "No match for " << augpath << endl;
  else
    DBG << "Multiple matches for " << augpath << endl;

  return string();
}

// ---------------------------------------------------------------------------

string Augeas::getOption(const string & option) const
{
  vector<string> opt;
  str::split(option, back_inserter(opt), "/");

  if (opt.size() != 2 || opt[0].empty() || opt[1].empty())
  {
    ERR << "invalid option " << option << endl;
    return string();
  }

  if (_got_user_zypper_conf)
  {
    string augpath_u = user_option_path(opt[0], opt[1], _homedir);
    string result = get(augpath_u);
    if (_last_get_result && !isCommented(opt[0], opt[1], false))
      return result;
  }

  if (_got_global_zypper_conf)
  {
    string augpath_g = global_option_path(opt[0], opt[1]);
    string result = get(augpath_g);
    if (_last_get_result && !isCommented(opt[0], opt[1], true))
      return result;
  }

  return string();
}

// ---------------------------------------------------------------------------

TriBool Augeas::isCommented(
    const string & section, const string & option, bool global) const
{
  // don't bother calling aug_get if we don't have the config read
  if ((global && !_got_global_zypper_conf) ||
      (!global && !_got_user_zypper_conf))
    return TriBool::indeterminate_value;

  Pathname path(global ?
      global_option_path(section, option) :
      user_option_path(section, option, _homedir));

  TriBool result;

  char ** matches;
  int matchcount = ::aug_match(_augeas, path.c_str(), &matches);
  if (matchcount == 1)
  {
    path = Pathname(matches[0]);
    // the 'commented' flag is a sibling of the key=value node
    path = path.dirname() + "/commented";
    DBG << path << ": ";
    int res = ::aug_get(_augeas, path.c_str(), NULL);
    if (res)
      result = true;
    else if (res == 0)
      result = false;
    DBG << result << endl;
  }
  for (int i = 0; i < matchcount; ++i)
    ::free(matches[i]);
  if (matchcount)
    ::free(matches);

  return result;
}

// ---------------------------------------------------------------------------

TriBool Augeas::isCommented(const string & option, bool global) const
{
  vector<string> opt;
  str::split(option, back_inserter(opt), "/");

  if (opt.size() != 2 || opt[0].empty() || opt[1].empty())
  {
    ERR << "invalid option " << option << endl;
    return TriBool::indeterminate_value;
  }

  return isCommented(opt[0], opt[1], global);
}
