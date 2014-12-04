/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <unordered_map>
extern "C"
{
  #include <libintl.h>
}

#include <zypp/base/Logger.h>
#include <zypp/base/Measure.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/ZConfig.h>

#include "utils/Augeas.h"
#include "Config.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

using namespace std;
using namespace zypp;

const ConfigOption ConfigOption::MAIN_SHOW_ALIAS(ConfigOption::MAIN_SHOW_ALIAS_e);
const ConfigOption ConfigOption::MAIN_REPO_LIST_COLUMNS(ConfigOption::MAIN_REPO_LIST_COLUMNS_e);
const ConfigOption ConfigOption::SOLVER_INSTALL_RECOMMENDS(ConfigOption::SOLVER_INSTALL_RECOMMENDS_e);
const ConfigOption ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS(ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS_e);
const ConfigOption ConfigOption::COLOR_USE_COLORS(ConfigOption::COLOR_USE_COLORS_e);
const ConfigOption ConfigOption::COLOR_RESULT(ConfigOption::COLOR_RESULT_e);
const ConfigOption ConfigOption::COLOR_MSG_STATUS(ConfigOption::COLOR_MSG_STATUS_e);
const ConfigOption ConfigOption::COLOR_MSG_ERROR(ConfigOption::COLOR_MSG_ERROR_e);
const ConfigOption ConfigOption::COLOR_MSG_WARNING(ConfigOption::COLOR_MSG_WARNING_e);
const ConfigOption ConfigOption::COLOR_POSITIVE(ConfigOption::COLOR_POSITIVE_e);
const ConfigOption ConfigOption::COLOR_CHANGE(ConfigOption::COLOR_CHANGE_e);
const ConfigOption ConfigOption::COLOR_NEGATIVE(ConfigOption::COLOR_NEGATIVE_e);
const ConfigOption ConfigOption::COLOR_INFO(ConfigOption::COLOR_INFO_e);
const ConfigOption ConfigOption::COLOR_HIGHLIGHT(ConfigOption::COLOR_HIGHLIGHT_e);
const ConfigOption ConfigOption::COLOR_PROMPT(ConfigOption::COLOR_PROMPT_e);
const ConfigOption ConfigOption::COLOR_PROMPT_SHORTHAND(ConfigOption::COLOR_PROMPT_SHORTHAND_e);
const ConfigOption ConfigOption::OBS_BASE_URL(ConfigOption::OBS_BASE_URL_e);
const ConfigOption ConfigOption::OBS_PLATFORM(ConfigOption::OBS_PLATFORM_e);

//////////////////////////////////////////////////////////////////
namespace
{
  typedef std::pair<std::string,ConfigOption::Option> OptionPair;
  /* add new options here: */
  const std::vector<OptionPair> & optionPairs()
  {
    static const std::vector<OptionPair> _data = {
      { "main/showAlias",			ConfigOption::MAIN_SHOW_ALIAS_e			},
      { "main/repoListColumns",			ConfigOption::MAIN_REPO_LIST_COLUMNS_e		},
      { "solver/installRecommends",		ConfigOption::SOLVER_INSTALL_RECOMMENDS_e	},
      { "solver/forceResolutionCommands",	ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS_e},
      { "color/useColors",			ConfigOption::COLOR_USE_COLORS_e		},
      { "color/result",				ConfigOption::COLOR_RESULT_e			},
      { "color/msgStatus",			ConfigOption::COLOR_MSG_STATUS_e		},
      { "color/msgError",			ConfigOption::COLOR_MSG_ERROR_e			},
      { "color/msgWarning",			ConfigOption::COLOR_MSG_WARNING_e		},
      { "color/positive",			ConfigOption::COLOR_POSITIVE_e			},
      { "color/change",				ConfigOption::COLOR_CHANGE_e			},
      { "color/negative",			ConfigOption::COLOR_NEGATIVE_e			},
      { "color/informational",			ConfigOption::COLOR_INFO_e			},
      { "color/highlight",			ConfigOption::COLOR_HIGHLIGHT_e			},
      { "color/prompt",				ConfigOption::COLOR_PROMPT_e			},
      { "obs/baseUrl",				ConfigOption::OBS_BASE_URL_e			},
      { "obs/platform",				ConfigOption::OBS_PLATFORM_e			}
    };
    return _data;
  }
} // namespace
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
namespace std
{
  template<>
  struct hash<ConfigOption::Option>
  {
    size_t operator()( const ConfigOption::Option & __s ) const
    { return __s; }
  };
} // namespace std
//////////////////////////////////////////////////////////////////

ConfigOption::ConfigOption(const std::string & strval_r)
  : _value(parse(strval_r))
{}

ConfigOption::Option ConfigOption::parse(const std::string & strval_r)
{
  static std::unordered_map<std::string,ConfigOption::Option> _index = [](){
    // index into optionPairs
    std::unordered_map<std::string,ConfigOption::Option> data;
    for ( const auto & p : optionPairs() )
      data[p.first] = p.second;
    return data;
  }();

  auto it( _index.find( strval_r ) );
  if ( it == _index.end() )
  {
    ZYPP_THROW(zypp::Exception(
      zypp::str::form(_("Unknown configuration option '%s'"), strval_r.c_str())));
  }
  return it->second;
}

string ConfigOption::asString() const
{
  static std::unordered_map<ConfigOption::Option,std::string> _index = [](){
    // index into optionPairs
    std::unordered_map<ConfigOption::Option,std::string> data;
    for ( const auto & p : optionPairs() )
      data[p.second] = p.first;
    return data;
  }();

  auto it( _index.find( _value ) );
  if ( it != _index.end() )
    return it->second;
  return string();
}


Config::Config()
  : repo_list_columns("anr")
  , solver_installRecommends(!ZConfig::instance().solver_onlyRequires())
  , do_colors        (false)
  , color_useColors  ("autodetect")
  , color_highlight  (true)
  , color_result     ("")
  , color_msgStatus  ("")
  , color_msgError   ("red")
  , color_msgWarning ("purple")
  , color_positive   ("green")
  , color_change     ("brown")
  , color_negative   ("red")
  , color_info       ("grey")
  , color_prompt     ("bold")
  , obs_baseUrl("http://download.opensuse.org/repositories/")
  , obs_platform("openSUSE_Factory")
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

    s = augeas.getOption(ConfigOption::MAIN_SHOW_ALIAS.asString());
    if (!s.empty())
    {
      // using Repository::asUserString() will follow repoLabelIsAlias!
      ZConfig::instance().repoLabelIsAlias( str::strToBool(s, false) );
    }

    s = augeas.getOption(ConfigOption::MAIN_REPO_LIST_COLUMNS.asString());
    if (!s.empty()) // TODO add some validation
      repo_list_columns = s;

    // ---------------[ solver ]------------------------------------------------

    s = augeas.getOption(ConfigOption::SOLVER_INSTALL_RECOMMENDS.asString());
    if (s.empty())
      solver_installRecommends = !ZConfig::instance().solver_onlyRequires();
    else
      solver_installRecommends = str::strToBool(s, true);

    s = augeas.getOption(ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS.asString());
    if (s.empty())
      solver_forceResolutionCommands.insert(ZypperCommand::REMOVE);
    else
    {
      list<string> cmdstr;
      str::split(s, std::back_inserter(cmdstr), ",");
      for_(c, cmdstr.begin(), cmdstr.end())
        solver_forceResolutionCommands.insert(ZypperCommand(str::trim(*c)));
    }


    // ---------------[ colors ]------------------------------------------------

    s = augeas.getOption(ConfigOption::COLOR_USE_COLORS.asString());
    if (!s.empty())
      color_useColors = s;
    do_colors =
      (color_useColors == "autodetect" && has_colors())
      || color_useColors == "always";

    Color c("");

    ////// color/colorResult //////

    c = Color(augeas.getOption(ConfigOption::COLOR_RESULT.asString()));
    if (!c.value().empty())
      color_result = c;

    ////// color/colorMsgStatus //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_STATUS.asString()));
    if (!c.value().empty())
      color_msgStatus = c;

    ////// color/colorMsgError //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_ERROR.asString()));
    if (!c.value().empty())
      color_msgError = c;

    ////// color/colorMsgWarning //////

    c = Color(augeas.getOption(ConfigOption::COLOR_MSG_WARNING.asString()));
    if (!c.value().empty())
      color_msgWarning = c;

    ////// color/colorPositive //////

    c = Color(augeas.getOption(ConfigOption::COLOR_POSITIVE.asString()));
    if (!c.value().empty())
      color_positive = c;

    ////// color/colorChange //////

    c = Color(augeas.getOption(ConfigOption::COLOR_CHANGE.asString()));
    if (!c.value().empty())
      color_change = c;

    ////// color/colorNegative //////

    c = Color(augeas.getOption(ConfigOption::COLOR_NEGATIVE.asString()));
    if (!c.value().empty())
      color_negative = c;

    ////// color/colorInformational //////

    c = Color(augeas.getOption(ConfigOption::COLOR_INFO.asString()));
    if (!c.value().empty())
      color_info = c;

    ////// color/highlight //////

    s = augeas.getOption(ConfigOption::COLOR_HIGHLIGHT.asString());
    if (!s.empty())
      color_highlight = s != "false" && s != "no";

    ////// color/colorPrompt //////

    c = Color(augeas.getOption(ConfigOption::COLOR_PROMPT.asString()));
    if (!c.value().empty())
      color_prompt = c;


    // ---------------[ obs ]---------------------------------------------------

    s = augeas.getOption(ConfigOption::OBS_BASE_URL.asString());
    if (!s.empty())
    {
      try { obs_baseUrl = Url(s); }
      catch (Exception & e)
      {
        ERR << "Invalid OBS base URL (" << e.msg() << "), will use the default." << endl;
      }
    }

    s = augeas.getOption(ConfigOption::OBS_PLATFORM.asString());
    if (!s.empty())
      obs_platform = s;

    m.stop();
  }
  catch (Exception & e)
  {
    std::cerr << e.asUserHistory() << endl;
    std::cerr << "*** Augeas exception. No config read, sticking with defaults." << endl;
  }
}
