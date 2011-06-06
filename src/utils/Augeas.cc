/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include <iostream>
#include <stdlib.h>

#include "zypp/base/Logger.h"
#include "zypp/Pathname.h"
#include "Zypper.h"
#include "utils/Augeas.h"

using namespace zypp;
using namespace std;

Augeas::Augeas(const string & file)
  : _augeas(NULL), _got_global_zypper_conf(false), _got_user_zypper_conf(false)
{
  MIL << "Going to read zypper config using Augeas..." << endl;

  // init
  _augeas = ::aug_init(NULL, "/usr/share/zypper", AUG_NO_STDINC | AUG_NO_LOAD);
  if (_augeas == NULL)
    ZYPP_THROW(Exception(_("Cannot initialize configuration file parser.")));

  // tell augeas which file to load
  // if file is specified, try to load the file, fall back to default files
  // default - /etc/zypp/zypper.conf & $HOME/.zypper.conf
  Pathname filepath(file);
  bool want_custom = false;
  if (!file.empty() && PathInfo(filepath).isExist())
  {
    want_custom = true;
    if (filepath.relative())
    {
      const char * env = ::getenv("PWD");
      string wd = env ? env : ".";
      filepath = wd / filepath;
    }

    if (::aug_set(_augeas, "/augeas/load/ZYpper/incl", filepath.asString().c_str()) != 0)
      ZYPP_THROW(Exception(_("Augeas error: setting config file to load failed.")));
  }
  else
  {
    const char * env = ::getenv("HOME");
    if ( env )
      _homedir = env;
    if (_homedir.empty())
      WAR << "Cannot figure out user's home directory. Skipping user's config." << endl;
    else
    {
      // add $HOME/.zypper.conf; /etc/zypp/zypper.conf should already be included
      filepath = _homedir + "/.zypper.conf";
      if (::aug_set(_augeas, "/augeas/load/ZYpper/incl[2]", filepath.asString().c_str()) != 0)
        ZYPP_THROW(Exception(_("Augeas error: setting config file to load failed.")));
    }
  }

  // load the file
  if (aug_load(_augeas) != 0)
    ZYPP_THROW(Exception(_("Could not parse the config files.")));

  // collect eventual errors
  const char *value[1] = {};
  string error;
  if (want_custom)
    _user_conf_path = "/files" + filepath.asString();
  else
  {
    if (!_homedir.empty())
      _user_conf_path = "/files" + _homedir + "/.zypper.conf";

    // global conf errors
    _got_global_zypper_conf =
      ::aug_get(_augeas, "/files/etc/zypp/zypper.conf", NULL) != 0;
    if (::aug_get(_augeas, "/augeas/files/etc/zypp/zypper.conf/error/message", value))
      error = value[0];

  }

  // user conf errors
  if (!_user_conf_path.empty())
  {
    _got_user_zypper_conf =
      ::aug_get(_augeas, _user_conf_path.c_str(), NULL) != 0;
    string user_conf_err = "/augeas" + _user_conf_path + "/error/message";
    if (::aug_get(_augeas, user_conf_err.c_str(), value))
      error += string("\n") + value[0];
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
  if (want_custom)
    MIL << "custom conf read: " << (_got_user_zypper_conf ? "yes" : "no") << endl;
  else
  {
    MIL << "user conf read: " << (_got_user_zypper_conf ? "yes" : "no") << endl;
    MIL << "global conf read: " << (_got_global_zypper_conf ? "yes" : "no") << endl;
  }
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

string Augeas::userOptionPath(
    const string & section, const string & option) const
{
  if (_user_conf_path.empty())
    return "(empty)";
  return _user_conf_path + "/" + section + "/*/" + option;
}

// ---------------------------------------------------------------------------

string Augeas::get(const string & augpath) const
{
  const char *value[1] = {};
  _last_get_result = ::aug_get(_augeas, augpath.c_str(), value);
  if (_last_get_result == 1)
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
    string augpath_u = userOptionPath(opt[0], opt[1]);
    string result = get(augpath_u);
    if (_last_get_result == 1 && !isCommented(opt[0], opt[1], false))
      return result;
  }

  if (_got_global_zypper_conf)
  {
    string augpath_g = global_option_path(opt[0], opt[1]);
    string result = get(augpath_g);
    if (_last_get_result == 1 && !isCommented(opt[0], opt[1], true))
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
      userOptionPath(section, option));

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
