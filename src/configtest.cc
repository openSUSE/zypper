/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>

#include "Zypper.h"
#include "configtest.h"

#include "Table.h"

using std::cout;
using std::endl;

///////////////////////////////////////////////////////////////////
/// ConfigtestOptions
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const ConfigtestOptions & obj )
{ return str << "ConfigtestOptions"; }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class Configtest
  /// \brief Implementation of configtest commands.
  ///////////////////////////////////////////////////////////////////
  class Configtest
  {
    typedef ConfigtestOptions Options;
  public:
    Configtest( Zypper & zypper_r )
    : _zypper( zypper_r )
     , _options( _zypper.commandOptionsAs<ConfigtestOptions>() )
    { MIL << "Configtest " << _options << endl; }

  public:
    void run();

  private:
    std::string exampleLine( ansi::Color color_r ) const
    {
      std::string sample( " Some example text. " );
      bool isDefault = ( color_r == ansi::Color::Default );
      if ( isDefault )
	*sample.begin() = *sample.rbegin() = '*';

      ColorStream str( color_r );
      str << sample
	  << ( ( isDefault ? ansi::Color(ansi::Color::White,ansi::Color::Bg::Black) : ansi::Color::Bg::Black ) << sample )
	  << ( ( isDefault ? ansi::Color(ansi::Color::Black,ansi::Color::Bg::White) : ansi::Color::Bg::White ) << sample );
      return str.str();
    }

  private:
    Zypper & _zypper;				//< my Zypper
    shared_ptr<ConfigtestOptions> _options;	//< my Options
  };
  ///////////////////////////////////////////////////////////////////

  void Configtest::run()
  {
    {
      PropertyTable p;
      for ( const auto & el : std::initializer_list<std::pair<const char *, ansi::Color>> {
	{ "black",		ansi::Color::Black		},
	{ "darkgrey",		ansi::Color::BrightBlack	},

	{ "red",		ansi::Color::Red		},
	{ "lightred",		ansi::Color::BrightRed		},

	{ "green",		ansi::Color::Green		},
	{ "lightgreen",		ansi::Color::BrightGreen	},

	{ "brown",		ansi::Color::Yellow		},
	{ "yellow",		ansi::Color::BrightYellow	},

	{ "blue",		ansi::Color::Blue		},
	{ "lightblue",		ansi::Color::BrightBlue		},

	{ "purple",		ansi::Color::Magenta		},
	{ "lightpurple",	ansi::Color::BrightMagenta	},

	{ "cyan",		ansi::Color::Cyan		},
	{ "lightcyan",		ansi::Color::BrightCyan		},

	{ "grey",		ansi::Color::White		},
	{ "white",		ansi::Color::BrightWhite	},

	{ "default",		ansi::Color::Default		},
	{ "lightdefault",	ansi::Color::BrightDefault	},
      } )
      {
	p.add( el.first, exampleLine(el.second) );
      }
      cout << "Known Colors:" << endl << p << endl;
    }
    {
      PropertyTable p;
      for ( const auto & el : std::initializer_list<std::pair<const char *, ansi::Color>> {
	{ "RESULT",		ColorContext::RESULT		},
	{ "MSG_STATUS",		ColorContext::MSG_STATUS	},
	{ "MSG_ERROR",		ColorContext::MSG_ERROR		},
	{ "MSG_WARNING",	ColorContext::MSG_WARNING	},
	{ "POSITIVE",		ColorContext::POSITIVE		},
	{ "NEGATIVE",		ColorContext::NEGATIVE		},
	{ "PROMPT_OPTION",	ColorContext::PROMPT_OPTION	},
	{ "HIGHLIGHT",		ColorContext::HIGHLIGHT		},
	{ "LOWLIGHT",		ColorContext::LOWLIGHT		},
	{ "OSDEBUG",		ColorContext::OSDEBUG		},
	{ "DEFAULT",		ColorContext::DEFAULT		},
      } )
      {
	p.add( el.first, exampleLine(el.second) );
      }
      cout << "Color Contexts:" << endl << p << endl;
    }
  }

} // namespace
///////////////////////////////////////////////////////////////////

int configtest( Zypper & zypper_r )
{
  try
  {
    Configtest( zypper_r ).run();
  }
  catch ( const Out::Error & error_r )
  {
    return error_r.report( zypper_r );
  }
  return zypper_r.exitCode();
}
