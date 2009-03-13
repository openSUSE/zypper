/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_CONFIG_H_
#define ZYPPER_CONFIG_H_

#include <string>

#include "utils/colors.h"

struct Config
{
  /** Initializes the config options to defaults. */
  Config();

  /** Reads zypper.conf and stores the result */
  void read();



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

  Color color_colorResult;
  Color color_colorMsgStatus;
  Color color_colorMsgError;
  Color color_colorMsgWarning;
  Color color_colorPositive;
  Color color_colorNegative;
  Color color_colorPromptOption;
  Color color_colorPromptShorthand;
};

#endif /* ZYPPER_CONFIG_H_ */
