/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_CONFIG_H_
#define ZYPPER_CONFIG_H_

#include <string>

struct Config
{
  /** Initializes the config options to defaults. */
  Config();

  /** Reads zypper.conf and stores the result */
  void read();



  /**
   * Whether to print colors to stdout. This is evaluated according to
   * color_useColors and has_colors()
   */
  bool do_colors;

  /** zypper.conf: color.useColors option */
  std::string color_useColors;

  /** dark (false) or light (true) */
  bool color_background;
};

#endif /* ZYPPER_CONFIG_H_ */
