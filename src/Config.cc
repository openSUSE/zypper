/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"

#include "Config.h"

using namespace std;
using namespace zypp;

Config::Config()
  : do_colors(false)
  , color_useColors("never")
  , color_background(false)          // dark background
  , color_colorResult     ("white")  // default colors for dark background
  , color_colorMsgStatus  ("grey")   // if background is actually light, these
  , color_colorMsgError   ("red")    // colors will be overwritten in read()
  , color_colorMsgWarning ("yellow")
  , color_colorPositive   ("green")
  , color_colorNegative   ("red")
  , color_colorPromptOption("grey")
  , color_colorPromptShorthand("yellow")
{}

void Config::read()
{
  debug::Measure m("ReadConfig");

  // get augeas

  m.elapsed();

  // ---------------[ main ]--------------------------------------------------

  // TODO

  // ---------------[ colors ]------------------------------------------------

  // color_useColors = augeas.getOption("colors/useColors");
  do_colors =
    (color_useColors == "autodetect" && has_colors())
    || color_useColors == "always";

  ////// colors/background //////

  string s;
  // s = augeas.getOption("colors/background");
  if (s == "light")
    color_background = true;
  else if (!s.empty() && s != "dark")
    ERR << "invalid colors/background value: " << s << endl;

  Color c("none");

  ////// colors/colorResult //////

  // c =  augeas.getOption("colors/colorResult");
  if (c.value().empty())
  {
    // set a default for light background
    if (color_background)
      color_colorResult = Color("black");
  }
  else
    color_colorResult = c;

  ////// colors/colorMsgStatus //////

  // c =  augeas.getOption("colors/colorMsgStatus");
  if (c.value().empty())
  {
    // set a default for light background
    if (color_background)
      color_colorMsgStatus = Color("default");
  }
  else
    color_colorMsgStatus = c;

  ////// colors/colorMsgError //////

  // c =  augeas.getOption("colors/colorMsgError");
  if (!c.value().empty())
    color_colorMsgError = c;

  ////// colors/colorMsgWarning //////

  // c =  augeas.getOption("colors/colorMsgWarning");
  if (c.value().empty())
  {
    // set a default for light background
    if (color_background)
      color_colorMsgWarning = Color("brown");
  }
  else
    color_colorMsgWarning = c;

  ////// colors/colorPositive //////

  // c =  augeas.getOption("colors/colorPositive");
  if (!c.value().empty())
    color_colorPositive = c;

  ////// colors/colorNegative //////

  // c =  augeas.getOption("colors/colorNegative");
  if (!c.value().empty())
    color_colorNegative = c;

  ////// colors/colorPromptOption //////

  // c =  augeas.getOption("colors/colorPromptOption");
  if (c.value().empty())
  {
    // set a default for light background
    if (color_background)
      color_colorPromptOption = Color("darkgrey");
  }
  else
    color_colorPromptOption = c;

  ////// colors/colorPromptShorthand //////

  // c =  augeas.getOption("colors/colorPromptShorthand");
  if (c.value().empty())
  {
    // set a default for light background
    if (color_background)
      color_colorPromptShorthand = Color("cyan");
  }
  else
    color_colorPromptShorthand = c;

  m.stop();
}
