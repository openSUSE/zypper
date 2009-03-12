/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "utils/colors.h"

#include "Config.h"

#include <iostream>

Config::Config()
  : do_colors(false)
  , color_useColors("never")
  , color_background(false)        // dark background
{}

void Config::read()
{
  // color_useColors = auges.getOption("colors/useColors");
  // ...
  do_colors =
    color_useColors == "autodetect" && has_colors()
    || color_useColors == "always";
}
