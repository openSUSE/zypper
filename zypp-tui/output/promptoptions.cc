/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include "promptoptions.h"

#include <zypp-core/base/Gettext.h>
#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

namespace ztui {

  // ----------------------------------------------------------------------------

  PromptOptions::PromptOptions( StrVector options_r, unsigned defaultOpt_r )
  { setOptions( std::move(options_r), defaultOpt_r ); }

  PromptOptions::PromptOptions( const std::string & optionstr_r, unsigned defaultOpt_r )
  { setOptions( optionstr_r, defaultOpt_r ); }

  // ----------------------------------------------------------------------------

  PromptOptions::~PromptOptions()
  {}

  // ----------------------------------------------------------------------------

  void PromptOptions::setOptions( StrVector options_r, unsigned defaultOpt_r )
  {
    _options.swap( options_r );
    if ( _options.size() <= defaultOpt_r )
    {
      INT << "Invalid default option index " << defaultOpt_r << std::endl;
      _default = 0;
    }
    else
      _default = defaultOpt_r;
  }

  void PromptOptions::setOptions( const std::string & optionstr_r, unsigned defaultOpt_r )
  {
    StrVector options;
    zypp::str::split( optionstr_r, back_inserter(options), "/" );
    setOptions( std::move(options), defaultOpt_r );
  }

  ColorString PromptOptions::optionString() const
  {
    bool hidden = false;	// have enabled options not shown at the prompt (/...)?
    unsigned shown = 0;
    unsigned showmax = ( _shown_count < 0 ? _options.size() : (unsigned)_shown_count );

    std::ostringstream str;
    str << "[";

    const char * slash = "";	// "/" after the 1st option
    for ( unsigned idx = 0; idx < _options.size(); ++idx )
    {
      if ( isDisabled(idx) )
        continue;

      if ( shown < showmax )
      {
        str << slash << ( ColorContext::PROMPT_OPTION << _options[idx] );
        if ( !shown ) slash = "/";
        ++shown;
      }
      else
      {
        hidden = true;
        break;	// don't mind how many
      }
    }

    if ( hidden || !_opt_help.empty() )
    {
      str << slash << ( hidden ? "..." : "" ) << ( ColorContext::PROMPT_OPTION << "?" );
      if ( hidden )
        // translators: Press '?' to see all options embedded in this prompt: "Continue? [y/n/? shows all options] (y):"
        str << " " << _("shows all options");
    }

    str << "]";

    if ( !_options.empty() )
      str << " (" << ( ColorContext::PROMPT_OPTION << _options[_default] ) << ")";

    return ColorString( str.str() );
  }


  void PromptOptions::setOptionHelp( unsigned opt, const std::string & help_str )
  {
    if ( help_str.empty() )
      return;

    if ( opt >= _options.size() )
    {
      WAR << "attempt to set option help for non-existing option."
          << " text: " << help_str << std::endl;
      return;
    }

    if ( opt >= _opt_help.capacity() )
      _opt_help.reserve( _options.size() );
    if ( opt >= _opt_help.size( ))
      _opt_help.resize( _options.size() );

    _opt_help[opt] = help_str;
  }

  std::vector<int> PromptOptions::getReplyMatches( const std::string & reply_r ) const
  {
    std::vector<int> ret;

    // #NUM ? (direct index into option vector)
    if ( reply_r[0] == '#' && reply_r[1] != '\0' )
    {
      unsigned num = 0;	// -1 : if no match
      for ( const char * cp = reply_r.c_str()+1; *cp; ++cp )
      {
        if ( '0' <= *cp && *cp <= '9' )
        {
          num *= 10;
          num += (*cp-'0');
        }
        else
        {
          num = unsigned(-1);
          break;
        }
      }

      if ( num != unsigned(-1) )
      {
        // userland counting! #1 is the 1st (enabled) option (#0 will never match)
        if ( num != 0 )
        {
          for ( unsigned i = 0; i < _options.size(); ++i )
          {
            if ( isDisabled(i) )
              continue;

            if ( --num == 0 )
            {
              ret.push_back( i );
              break;
            }
          }
        }
        return ret;	// a match - good or bad - will be eaten
      }
      // no match falls through....
    }

    const std::string & lreply { zypp::str::toLower( reply_r ) };
    for ( unsigned i = 0; i < _options.size(); ++i )
    {
      if ( isDisabled(i) )
        continue;

      const std::string & lopt { zypp::str::toLower( _options[i] ) };

      if ( lopt == lreply ) {	// prefer an exact match ("1/11")
        ret.clear();
        ret.push_back( i );
        break;
      }
      else if ( zypp::str::hasPrefix( lopt, lreply ) )
        ret.push_back( i );
    }

    return ret;
  }

  std::string PromptOptions::replyMatchesStr( const std::vector<int> & matches_r ) const
  {
    zypp::str::Str str;
    const char * sep = "(";	// "," after the 1st option
    for ( unsigned idx : matches_r )
    {
      str << sep << _options[idx];
      if ( *sep != ',' ) sep =",";
    }
    return str << ")";
  }

  bool PromptOptions::isYesNoPrompt() const
  { return _options.size() == 2 && _options[0] == _("yes") && _options[1] == _("no"); }

}
