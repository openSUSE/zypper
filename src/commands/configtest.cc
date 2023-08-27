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

namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class Configtest
  /// \brief Implementation of configtest commands.
  ///////////////////////////////////////////////////////////////////
  class Configtest
  {
  public:
    Configtest( )
    { MIL << "Configtest " << endl; }

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
        { "PROMPT",		ColorContext::PROMPT		},
        { "PROMPT_OPTION",	ColorContext::PROMPT_OPTION	},
        { "POSITIVE",		ColorContext::POSITIVE		},
        { "CHANGE",		ColorContext::CHANGE		},
        { "NEGATIVE",		ColorContext::NEGATIVE		},
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


ConfigTestCmd::ConfigTestCmd(std::vector<std::string> &&commandAliases_r):
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    "configtest",
    "This command is for debugging purposes only.",
    "This command is for debugging purposes only.",
    DisableAll
        )
{ }

int ConfigTestCmd::execute(Zypper &, const std::vector<std::string> &)
{
  // Configtest debug command
  Configtest().run();
  return ZYPPER_EXIT_OK;
}
