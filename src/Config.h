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

class ConfigOption
{
public:
  static const ConfigOption MAIN_SHOW_ALIAS;
  static const ConfigOption MAIN_REPO_LIST_COLUMNS;

  static const ConfigOption SOLVER_INSTALL_RECOMMENDS;
  static const ConfigOption SOLVER_FORCE_RESOLUTION_COMMANDS;

  static const ConfigOption COLOR_USE_COLORS;
  static const ConfigOption COLOR_BACKGROUND;
  static const ConfigOption COLOR_RESULT;
  static const ConfigOption COLOR_MSG_STATUS;
  static const ConfigOption COLOR_MSG_ERROR;
  static const ConfigOption COLOR_MSG_WARNING;
  static const ConfigOption COLOR_POSITIVE;
  static const ConfigOption COLOR_NEGATIVE;
  static const ConfigOption COLOR_HIGHLIGHT;
  static const ConfigOption COLOR_PROMPT_OPTION;
  static const ConfigOption COLOR_PROMPT_SHORTHAND;

  static const ConfigOption OBS_BASE_URL;
  static const ConfigOption OBS_PLATFORM;

  enum Option
  {
    MAIN_SHOW_ALIAS_e,
    MAIN_REPO_LIST_COLUMNS_e,

    SOLVER_INSTALL_RECOMMENDS_e,
    SOLVER_FORCE_RESOLUTION_COMMANDS_e,

    COLOR_USE_COLORS_e,
    COLOR_BACKGROUND_e,
    COLOR_RESULT_e,
    COLOR_MSG_STATUS_e,
    COLOR_MSG_ERROR_e,
    COLOR_MSG_WARNING_e,
    COLOR_POSITIVE_e,
    COLOR_NEGATIVE_e,
    COLOR_HIGHLIGHT_e,
    COLOR_PROMPT_OPTION_e,
    COLOR_PROMPT_SHORTHAND_e,

    OBS_BASE_URL_e,
    OBS_PLATFORM_e
  };

  ConfigOption(Option option) : _value(option) {}

  explicit ConfigOption(const std::string & strval_r);

  Option toEnum() const { return _value; }

  ConfigOption::Option parse(const std::string & strval_r);

  std::string asString() const;

private:
  Option _value;
};


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

  /**
   * zypper.conf: color.background
   * dark (false) or light (true)
   */
  bool color_background;

  Color color_result;
  Color color_msgStatus;
  Color color_msgError;
  Color color_msgWarning;
  Color color_positive;
  Color color_negative;
  Color color_highlight;
  Color color_promptOption;

  /** zypper.conf: obs.baseUrl */
  zypp::Url obs_baseUrl;
  /** zypper.conf: obs.platform */
  std::string obs_platform;
};

#endif /* ZYPPER_CONFIG_H_ */
