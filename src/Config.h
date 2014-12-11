/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_CONFIG_H_
#define ZYPPER_CONFIG_H_

#include <string>
#include <set>

#include <zypp/Url.h>

#include "Command.h"
#include "utils/colors.h"

/**
 *
 */
struct Config
{
  /** Initializes the config options to defaults. */
  Config();

  /** Reads zypper.conf and stores the result */
  void read(const std::string & file = "");

  /** Which columns to show in repo list by default (string of short options).*/
  std::string repo_list_columns;

  bool solver_installRecommends;
  std::set<ZypperCommand> solver_forceResolutionCommands;

  /**
   * Whether to colorize the output. This is evaluated according to
   * color_useColors and has_colors()
   */
  bool do_colors;

  /** zypper.conf: color.useColors */
  std::string color_useColors;

  ansi::Color color_result;
  ansi::Color color_msgStatus;
  ansi::Color color_msgError;
  ansi::Color color_msgWarning;
  ansi::Color color_prompt;
  ansi::Color color_promptOption;
  ansi::Color color_positive;
  ansi::Color color_change;
  ansi::Color color_negative;
  ansi::Color color_highlight;
  ansi::Color color_lowlight;
  ansi::Color color_osdebug;

  zypp::TriBool color_pkglistHighlight;	// true:all; indeterminate:first; false:no

  /** zypper.conf: obs.baseUrl */
  zypp::Url obs_baseUrl;
  /** zypper.conf: obs.platform */
  std::string obs_platform;
};

#endif /* ZYPPER_CONFIG_H_ */
