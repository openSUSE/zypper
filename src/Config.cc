/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <map>
extern "C"
{
  #include <libintl.h>
}

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"
#include "zypp/ZConfig.h"

#include "utils/Augeas.h"
#include "Config.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

using namespace std;
using namespace zypp;

static map<string, ConfigOption::Option> _table;
static map<ConfigOption::Option, string> _table_str;
const ConfigOption ConfigOption::SOLVER_INSTALL_RECOMMENDS(ConfigOption::SOLVER_INSTALL_RECOMMENDS_e);
const ConfigOption ConfigOption::COLOR_USE_COLORS(ConfigOption::COLOR_USE_COLORS_e);
const ConfigOption ConfigOption::COLOR_BACKGROUND(ConfigOption::COLOR_BACKGROUND_e);
const ConfigOption ConfigOption::COLOR_RESULT(ConfigOption::COLOR_RESULT_e);
const ConfigOption ConfigOption::COLOR_MSG_STATUS(ConfigOption::COLOR_MSG_STATUS_e);
const ConfigOption ConfigOption::COLOR_MSG_ERROR(ConfigOption::COLOR_MSG_ERROR_e);
const ConfigOption ConfigOption::COLOR_MSG_WARNING(ConfigOption::COLOR_MSG_WARNING_e);
const ConfigOption ConfigOption::COLOR_POSITIVE(ConfigOption::COLOR_POSITIVE_e);
const ConfigOption ConfigOption::COLOR_NEGATIVE(ConfigOption::COLOR_NEGATIVE_e);
const ConfigOption ConfigOption::COLOR_HIGHLIGHT(ConfigOption::COLOR_HIGHLIGHT_e);
const ConfigOption ConfigOption::COLOR_PROMPT_OPTION(ConfigOption::COLOR_PROMPT_OPTION_e);
const ConfigOption ConfigOption::COLOR_PROMPT_SHORTHAND(ConfigOption::COLOR_PROMPT_SHORTHAND_e);

ConfigOption::ConfigOption(const std::string & strval_r)
  : _value(parse(strval_r))
{}

ConfigOption::Option ConfigOption::parse(const std::string & strval_r)
{
  if (_table.empty())
  {
    // initialize it
    _table["solver/installRecommends"] = ConfigOption::SOLVER_INSTALL_RECOMMENDS_e;
    _table["color/useColors"] = ConfigOption::COLOR_USE_COLORS_e;
    _table["color/background"] = ConfigOption::COLOR_BACKGROUND_e;
    _table["color/result"] = ConfigOption::COLOR_RESULT_e;
    _table["color/msgStatus"] = ConfigOption::COLOR_MSG_STATUS_e;
    _table["color/msgError"] = ConfigOption::COLOR_MSG_ERROR_e;
    _table["color/msgWarning"] = ConfigOption::COLOR_MSG_WARNING_e;
    _table["color/positive"] = ConfigOption::COLOR_POSITIVE_e;
    _table["color/negative"] = ConfigOption::COLOR_NEGATIVE_e;
    _table["color/highlight"] = ConfigOption::COLOR_HIGHLIGHT_e;
    _table["color/promptOption"] = ConfigOption::COLOR_PROMPT_OPTION_e;
  }
  map<string, ConfigOption::Option>::const_iterator it = _table.find(strval_r);
  if (it == _table.end())
  {
    string message =
      zypp::str::form(_("Unknown configuration option '%s'"), strval_r.c_str());
    ZYPP_THROW(zypp::Exception(message));
  }
  return it->second;
}

const string ConfigOption::asString() const
{
  if (_table.empty())
  {
    // initialize it
    _table_str[SOLVER_INSTALL_RECOMMENDS_e] = string("solver/installRecommends");
    _table_str[COLOR_USE_COLORS_e] = string("color/useColors");
    _table_str[COLOR_BACKGROUND_e] = "color/background";
    _table_str[COLOR_RESULT_e] = "color/result";
    _table_str[COLOR_MSG_STATUS_e] = "color/msgStatus";
    _table_str[COLOR_MSG_ERROR_e] = "color/msgError";
    _table_str[COLOR_MSG_WARNING_e] = "color/msgWarning";
    _table_str[COLOR_POSITIVE_e] = "color/positive";
    _table_str[COLOR_NEGATIVE_e] = "color/negative";
    _table_str[COLOR_HIGHLIGHT_e] = "color/highlight";
    _table_str[COLOR_PROMPT_OPTION_e] = "color/promptOption";
  }
  map<ConfigOption::Option, string>::const_iterator it = _table_str.find(_value);
  if (it != _table_str.end())
    return it->second;
  return string();
}


Config::Config()
  : solver_installRecommends(true)
  , do_colors        (false)
  , color_useColors  ("never")
  , color_background (false)    // dark background
  , color_result     ("white")  // default colors for dark background
  , color_msgStatus  ("grey")   // if background is actually light, these
  , color_msgError   ("red")    // colors will be overwritten in read()
  , color_msgWarning ("yellow")
  , color_positive   ("green")
  , color_negative   ("red")
  , color_highlight  ("cyan")
  , color_promptOption("grey")
{}

void Config::read(const string & file)
{
  try
  {
    debug::Measure m("ReadConfig");
    string s;

    Augeas augeas(file);

    m.elapsed();

    // ---------------[ main ]--------------------------------------------------

    // TODO

    // ---------------[ main ]--------------------------------------------------

    s = augeas.getOption(ConfigOption::SOLVER_INSTALL_RECOMMENDS.asString());
    if (s.empty())
      solver_installRecommends = !ZConfig::instance().solver_onlyRequires();
    else
      solver_installRecommends = str::strToBool(s, true);


    // ---------------[ colors ]------------------------------------------------

    color_useColors = augeas.getOption(ConfigOption::COLOR_USE_COLORS.asString());
    do_colors =
      (color_useColors == "autodetect" && has_colors())
      || color_useColors == "always";

    ////// color/background //////

    s = augeas.getOption(ConfigOption::COLOR_BACKGROUND.asString());
    if (s == "light")
      color_background = true;
    else if (!s.empty() && s != "dark")
      ERR << "invalid color/background value: " << s << endl;

    Color c("");

    ////// color/colorResult //////

    c = Color(augeas.getOption(ConfigOption::COLOR_RESULT.asString()));
    if (c.value().empty())
    {
      // set a default for light background
      if (color_background)
        color_result = Color("black");
    }
    else
      color_result = c;

    ////// color/colorMsgStatus //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_STATUS.asString()));
    if (c.value().empty())
    {
      // set a default for light background
      if (color_background)
        color_msgStatus = Color("default");
    }
    else
      color_msgStatus = c;

    ////// color/colorMsgError //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_ERROR.asString()));
    if (!c.value().empty())
      color_msgError = c;

    ////// color/colorMsgWarning //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_WARNING.asString()));
    if (c.value().empty())
    {
      // set a default for light background
      if (color_background)
        color_msgWarning = Color("brown");
    }
    else
      color_msgWarning = c;

    ////// color/colorPositive //////

    c = Color(augeas.getOption(ConfigOption::COLOR_POSITIVE.asString()));
    if (!c.value().empty())
      color_positive = c;

    ////// color/colorNegative //////

    c = Color(augeas.getOption(ConfigOption::COLOR_NEGATIVE.asString()));
    if (!c.value().empty())
      color_negative = c;

    ////// color/highlight //////

    c = Color(augeas.getOption(ConfigOption::COLOR_HIGHLIGHT.asString()));
    if (!c.value().empty())
      color_highlight = c;

    ////// color/colorPromptOption //////

    c = Color(augeas.getOption(ConfigOption::COLOR_PROMPT_OPTION.asString()));
    if (c.value().empty())
    {
      // set a default for light background
      if (color_background)
        color_promptOption = Color("darkgrey");
    }
    else
      color_promptOption = c;

    m.stop();
  }
  catch (Exception & e)
  {
    DBG << "Augeas exception. No config read, sticking with defaults." << endl;
  }
}
