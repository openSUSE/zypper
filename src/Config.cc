/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

extern "C"
{
  #include <libintl.h>
}
#include <iostream>

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

using namespace zypp;
using std::endl;

//////////////////////////////////////////////////////////////////
namespace
{
  /** Color names (case insensitive) accepted in the config file. */
  ansi::Color namedColor( std::string name_r )
  {
    static const std::map<std::string, ansi::Color> _def =  {
      { "black",	ansi::Color::Black		},
      { "darkgrey",	ansi::Color::BrightBlack	},
      { "red",		ansi::Color::Red		},
      { "green",	ansi::Color::Green		},
      { "brown",	ansi::Color::Yellow		},
      { "yellow",	ansi::Color::BrightYellow	},
      { "blue",		ansi::Color::Blue		},
      { "magenta",	ansi::Color::Magenta		},
      { "purple",	ansi::Color::Magenta		},
      { "cyan",		ansi::Color::Cyan		},
      { "grey",		ansi::Color::White		},
      { "white",	ansi::Color::BrightWhite	},
      { "default",	ansi::Color::Default		},
      { "",		ansi::Color::Default		},  // matches "bold" "light" "bright" NOT ""
    };

    ansi::Color ret = ansi::Color::nocolor();

    if ( name_r.empty() )	// "" when undefined in config file
      return ret;

    name_r = str::toLower( name_r );

    if ( str::hasPrefix( name_r, "bold" ) )
    {
      name_r.erase( 0, 4 );
      ret <= ansi::Color::Attr::Bright;
    }
    else if (str::hasPrefix( name_r, "light" ) )
    {
      name_r.erase( 0, 5 );
      ret <= ansi::Color::Attr::Bright;
    }
    else if (str::hasPrefix( name_r, "bright" ) )
    {
      name_r.erase( 0, 6 );
      ret <= ansi::Color::Attr::Bright;
    }

    auto && it = _def.find( name_r );
    if ( it == _def.end() )
    {
      ERR << "Unknown color name '" << name_r << "'" << endl;
      ret = ansi::Color::Default;
    }
    else
    {
      ret = ( it->second < ret );
    }
    return ret;
  }
} // namespace
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
namespace
{
  enum class ConfigOption {
    MAIN_SHOW_ALIAS,
    MAIN_REPO_LIST_COLUMNS,

    SOLVER_INSTALL_RECOMMENDS,
    SOLVER_FORCE_RESOLUTION_COMMANDS,

    COLOR_USE_COLORS,
    COLOR_RESULT,
    COLOR_MSG_STATUS,
    COLOR_MSG_ERROR,
    COLOR_MSG_WARNING,
    COLOR_PROMPT,
    COLOR_PROMPT_OPTION,
    COLOR_POSITIVE,
    COLOR_CHANGE,
    COLOR_NEGATIVE,
    COLOR_HIGHLIGHT,
    COLOR_LOWLIGHT,
    COLOR_OSDEBUG,

    COLOR_PKGLISTHIGHLIGHT,
    COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE,

    OBS_BASE_URL,
    OBS_PLATFORM
  };

  const std::vector<std::pair<std::string,ConfigOption>> & optionPairs()
  {
    static const std::vector<std::pair<std::string,ConfigOption>> _data = {
      { "main/showAlias",			ConfigOption::MAIN_SHOW_ALIAS			},
      { "main/repoListColumns",			ConfigOption::MAIN_REPO_LIST_COLUMNS		},
      { "solver/installRecommends",		ConfigOption::SOLVER_INSTALL_RECOMMENDS		},
      { "solver/forceResolutionCommands",	ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS	},

      { "color/useColors",			ConfigOption::COLOR_USE_COLORS			},
      //"color/background"			LEGACY
      { "color/result",				ConfigOption::COLOR_RESULT			},
      { "color/msgStatus",			ConfigOption::COLOR_MSG_STATUS			},
      { "color/msgError",			ConfigOption::COLOR_MSG_ERROR			},
      { "color/msgWarning",			ConfigOption::COLOR_MSG_WARNING			},
      { "color/prompt",				ConfigOption::COLOR_PROMPT			},
      { "color/promptOption",			ConfigOption::COLOR_PROMPT_OPTION		},
      { "color/positive",			ConfigOption::COLOR_POSITIVE			},
      { "color/change",				ConfigOption::COLOR_CHANGE			},
      { "color/negative",			ConfigOption::COLOR_NEGATIVE			},
      { "color/highlight",			ConfigOption::COLOR_HIGHLIGHT			},
      { "color/lowlight",			ConfigOption::COLOR_LOWLIGHT			},
      { "color/osdebug",			ConfigOption::COLOR_OSDEBUG			},

      { "color/pkglistHighlight",		ConfigOption::COLOR_PKGLISTHIGHLIGHT		},
      { "color/pkglistHighlightAttribute",	ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE	},

      { "obs/baseUrl",				ConfigOption::OBS_BASE_URL			},
      { "obs/platform",				ConfigOption::OBS_PLATFORM			}
    };
    return _data;
  }

  std::string asString( ConfigOption value_r )
  {
    for ( const auto & p : optionPairs() )
    {
      if ( p.second == value_r )
	return p.first;
    }
    return std::string();
  }

} // namespace
//////////////////////////////////////////////////////////////////

Config::Config()
  : repo_list_columns("anr")
  , solver_installRecommends(!ZConfig::instance().solver_onlyRequires())
  , do_colors		(false)
  , color_useColors	("autodetect")
  , color_result	(namedColor("default"))
  , color_msgStatus	(namedColor("default"))
  , color_msgError	(namedColor("red"))
  , color_msgWarning	(namedColor("purple"))
  , color_prompt	(namedColor("bold"))
  , color_promptOption	(ansi::Color::nocolor())	// follow color_prompt
  , color_positive	(namedColor("green"))
  , color_change	(namedColor("brown"))
  , color_negative	(namedColor("red"))
  , color_highlight	(namedColor("cyan"))
  , color_lowlight	(namedColor("brown"))
  , color_osdebug	(namedColor("default") < ansi::Color::Attr::Reverse)
  , color_pkglistHighlight(true)
  , color_pkglistHighlightAttribute(ansi::Color::nocolor())
  , obs_baseUrl("http://download.opensuse.org/repositories/")
  , obs_platform("")	// guess 
{}

void Config::read( const std::string & file )
{
  try
  {
    debug::Measure m("ReadConfig");
    std::string s;

    Augeas augeas(file);

    m.elapsed();

    // ---------------[ main ]--------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::MAIN_SHOW_ALIAS ));
    if (!s.empty())
    {
      // using Repository::asUserString() will follow repoLabelIsAlias!
      ZConfig::instance().repoLabelIsAlias( str::strToBool(s, false) );
    }

    s = augeas.getOption(asString( ConfigOption::MAIN_REPO_LIST_COLUMNS ));
    if (!s.empty()) // TODO add some validation
      repo_list_columns = s;

    // ---------------[ solver ]------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::SOLVER_INSTALL_RECOMMENDS ));
    if (s.empty())
      solver_installRecommends = !ZConfig::instance().solver_onlyRequires();
    else
      solver_installRecommends = str::strToBool(s, true);

    s = augeas.getOption(asString( ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS ));
    if (s.empty())
      solver_forceResolutionCommands.insert(ZypperCommand::REMOVE);
    else
    {
      std::list<std::string> cmdstr;
      str::split(s, std::back_inserter(cmdstr), ",");
      for_(c, cmdstr.begin(), cmdstr.end())
        solver_forceResolutionCommands.insert(ZypperCommand(str::trim(*c)));
    }


    // ---------------[ colors ]------------------------------------------------

    s = augeas.getOption( asString( ConfigOption::COLOR_USE_COLORS ) );
    if (!s.empty())
      color_useColors = s;

    do_colors = ( color_useColors == "autodetect" && has_colors() ) || color_useColors == "always";

    ansi::Color c;
    for ( const auto & el : std::initializer_list<std::pair<ansi::Color &, ConfigOption>> {
      { color_result,		ConfigOption::COLOR_RESULT		},
      { color_msgStatus,	ConfigOption::COLOR_MSG_STATUS		},
      { color_msgError,		ConfigOption::COLOR_MSG_ERROR		},
      { color_msgWarning,	ConfigOption::COLOR_MSG_WARNING		},
      { color_prompt,		ConfigOption::COLOR_PROMPT		},
      { color_promptOption,	ConfigOption::COLOR_PROMPT_OPTION	},
      { color_positive,		ConfigOption::COLOR_POSITIVE		},
      { color_change,		ConfigOption::COLOR_CHANGE		},
      { color_negative,		ConfigOption::COLOR_NEGATIVE		},
      { color_highlight,	ConfigOption::COLOR_HIGHLIGHT		},
      { color_lowlight,		ConfigOption::COLOR_LOWLIGHT		},
      { color_pkglistHighlightAttribute, ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE },
    } )
    {
      c = namedColor( augeas.getOption( asString( el.second ) ) );
      if ( c )
	el.first = c;
      // Fix color attributes: Default is mapped to Unchanged to allow
      // using the ColorStreams default rather than the terminal default.
      if ( el.second == ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE )	// currently the only one
      {
	ansi::Color & c( el.first );
	if ( c.fg() == ansi::Color::Fg::Default )
	  c.fg( ansi::Color::Fg::Unchanged );
	if ( c.bg() == ansi::Color::Bg::Default )
	  c.bg( ansi::Color::Bg::Unchanged );
      }
    }

    s = augeas.getOption( asString( ConfigOption::COLOR_PKGLISTHIGHLIGHT ) );
    if (!s.empty())
    {
      if ( s == "all" )
	color_pkglistHighlight = true;
      else if ( s == "first" )
	color_pkglistHighlight = indeterminate;
      else if ( s == "no" )
	color_pkglistHighlight = false;
      else
	WAR << "zypper.conf: color/pkglistHighlight: unknown value '" << s << "'" << endl;
    }

    s = augeas.getOption("color/background");	// legacy
    if ( !s.empty() )
      WAR << "zypper.conf: ignore legacy option 'color/background'" << endl;

    // ---------------[ obs ]---------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::OBS_BASE_URL ));
    if (!s.empty())
    {
      try { obs_baseUrl = Url(s); }
      catch (Exception & e)
      {
        ERR << "Invalid OBS base URL (" << e.msg() << "), will use the default." << endl;
      }
    }

    s = augeas.getOption(asString( ConfigOption::OBS_PLATFORM ));
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
