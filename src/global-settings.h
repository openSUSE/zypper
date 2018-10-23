/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef GLOBAL_SETTINGS_H_INCLUDED
#define GLOBAL_SETTINGS_H_INCLUDED

#include <vector>
#include <string>

/**
 * \file This file is used for settings and flags that were directly parsed from copts in the code
 * \todo revisit each and every structure to determine if it can be replaced by a more clean approach
 */

namespace GlobalSettings
{
  void reset ();
}

template <typename T>
struct GlobalSettingSingleton
{
  static const T &instance () {
    return instanceNoConst();
  }
  static T &instanceNoConst () {
    static T me;
    return me;
  }
  static void reset () {
    instanceNoConst().reset();
  }
};

/**
 * Specifies if zypper should operate in dry-run mode
 */
struct DryRunSettingsData
{
  inline bool isEnabled () const {
    return _enabled;
  }

  void reset();

  bool _enabled = false;
};
using DryRunSettings = GlobalSettingSingleton<DryRunSettingsData>;

/**
 * Influences how zypper initializes the repositories when init_repos is called
 */
struct InitRepoSettingsData {
  void reset();

  std::vector<std::string> _repoFilter;
};
using InitRepoSettings = GlobalSettingSingleton<InitRepoSettingsData>;



#endif
