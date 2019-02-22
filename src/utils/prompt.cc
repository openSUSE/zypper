/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <ctype.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <poll.h>
#include <readline/readline.h>
#include <termios.h>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

#include "Zypper.h"
#include "utils/colors.h"

#include "prompt.h"

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
    INT << "Invalid default option index " << defaultOpt_r << endl;
    _default = 0;
  }
  else
    _default = defaultOpt_r;
}

void PromptOptions::setOptions( const std::string & optionstr_r, unsigned defaultOpt_r )
{
  StrVector options;
  str::split( optionstr_r, back_inserter(options), "/" );
  setOptions( std::move(options), defaultOpt_r );
}

ColorString PromptOptions::optionString() const
{
  std::ostringstream str;

  unsigned shown = 0;
  unsigned maxidx = _shown_count < 0 ? _options.size()
                                     : ( (unsigned)_shown_count < _options.size() ? _shown_count : _options.size() );
  for ( unsigned idx = 0; idx < maxidx; ++idx )
  {
    if ( isEnabled( idx ) )
    {
      str << ( shown ? "/" : "[" ) << ( ColorContext::PROMPT_OPTION << _options[idx] );
      ++shown;
    }
  }

  if ( !_opt_help.empty() )
  {
    if ( shown )
      str << ( maxidx != _options.size() ? "/..." : "/" );
    // translators: Press '?' to see all options embedded in this prompt: "Continue? [y/n/? shows all options] (y):"
    str << ( ColorContext::PROMPT_OPTION << "?" ) << " " << _("shows all options");
  }

  if ( shown )
  {
    str << "] (" << ( ColorContext::PROMPT_OPTION << _options[_default] ) << ")";
  }

  return ColorString( str.str() );
}


void PromptOptions::setOptionHelp( unsigned opt, const std::string & help_str )
{
  if ( help_str.empty() )
    return;

  if ( opt >= _options.size() )
  {
    WAR << "attempt to set option help for non-existing option."
        << " text: " << help_str << endl;
    return;
  }

  if ( opt >= _opt_help.capacity() )
    _opt_help.reserve( _options.size() );
  if ( opt >= _opt_help.size( ))
    _opt_help.resize( _options.size() );

  _opt_help[opt] = help_str;
}


int PromptOptions::getReplyIndex( const std::string & reply ) const
{
  DBG << " reply: " << reply << " (" << str::toLower(reply) << " lowercase)" << endl;

  unsigned i = 0;
  for( StrVector::const_iterator it = _options.begin(); it != _options.end(); ++i, ++it )
  {
    DBG << "index: " << i << " option: " << *it << endl;
    if ( *it == str::toLower(reply) )
    {
      if ( isDisabled(i) )
        break;
      return i;
    }
  }

  return -1;
}

bool PromptOptions::isYesNoPrompt() const
{ return _options.size() == 2 && _options[0] == _("yes") && _options[1] == _("no"); }

// ----------------------------------------------------------------------------

//! \todo The default values seems to be useless - we always want to auto-return 'retry' in case of no user input.
int read_action_ari_with_timeout( PromptId pid, unsigned timeout, int default_action )
{
  Zypper & zypper( Zypper::instance() );

  if ( default_action > 2 || default_action < 0 )
  {
    WAR << pid << " bad default action" << endl;
    default_action = 0;
  }

  // wait 'timeout' number of seconds and return the default in non-interactive mode
  if ( zypper.config().non_interactive )
  {
    zypper.out().info( str::form(_("Retrying in %u seconds..."), timeout) );
    sleep( timeout );
    MIL << pid << " running non-interactively, returning " << default_action << endl;
    return default_action;
  }

  PromptOptions poptions(_("a/r/i"), (unsigned) default_action );
  zypper.out().prompt( pid, _("Abort, retry, ignore?"), poptions );
  cout << endl;

  while ( timeout )
  {
    char reply = 0;
    unsigned reply_int = (unsigned)default_action;
    pollfd pollfds;
    pollfds.fd = 0; // stdin
    pollfds.events = POLLIN; // wait only for data to read

    //! \todo poll() reports the file is ready only after it contains newline
    //!       is there a way to do this without waiting for newline?
    while ( poll( &pollfds, 1, 5 ) ) // some user input, timeout 5msec
    {
      reply = getchar();
      char reply_str[2] = { reply, 0 };
      DBG << pid << " reply: " << reply << " (" << str::toLower(reply_str) << " lowercase)" << endl;
      bool got_valid_reply = false;
      for ( unsigned i = 0; i < poptions.options().size(); i++ )
      {
        DBG << pid << " index: " << i << " option: " << poptions.options()[i] << endl;
        if ( poptions.options()[i] == str::toLower(reply_str) )
        {
          reply_int = i;
          got_valid_reply = true;
          break;
        }
      }

      if ( got_valid_reply )
      {
        // eat the rest of input
        do {} while ( getchar() != '\n' );
        return reply_int;
      }
      else if ( feof(stdin) )
      {
        zypper.out().info( str::form(_("Retrying in %u seconds..."), timeout) );
        WAR << pid << " no good input, returning " << default_action
          << " in " << timeout << " seconds." << endl;
        sleep( timeout );
        return default_action;
      }
      else
        WAR << pid << " Unknown char " << reply << endl;
    }

    std::string msg = str::Format(PL_("Autoselecting '%s' after %u second.",
				      "Autoselecting '%s' after %u seconds.",
				      timeout)) % poptions.options()[default_action] % timeout;

    if ( zypper.out().type() == Out::TYPE_XML )
      zypper.out().info( msg );	// maybe progress??
    else
    {
      cout << ansi::tty::clearLN << msg << " ";
      cout.flush();
    }

    sleep( 1 );
    --timeout;
  }

  if ( zypper.out().type() != Out::TYPE_XML )
    cout << ansi::tty::clearLN << _("Trying again...") << endl;

  return default_action;
}


// ----------------------------------------------------------------------------
//template<typename Action>
//Action ...
int read_action_ari( PromptId pid, int default_action )
{
  Zypper & zypper( Zypper::instance() );
  // translators: "a/r/i" are the answers to the
  // "Abort, retry, ignore?" prompt
  // Translate the letters to whatever is suitable for your language.
  // the answers must be separated by slash characters '/' and must
  // correspond to abort/retry/ignore in that order.
  // The answers should be lower case letters.
  PromptOptions popts(_("a/r/i"), (unsigned) default_action );
  if ( !zypper.config().non_interactive )
    clear_keyboard_buffer();
  zypper.out().prompt( pid, _("Abort, retry, ignore?"), popts );
  return get_prompt_reply( zypper, pid, popts );
}

// ----------------------------------------------------------------------------

bool read_bool_answer( PromptId pid, const std::string & question, bool default_answer )
{
  Zypper & zypper( Zypper::instance() );
  std::string yn( std::string(_("yes")) + "/" + _("no") );
  PromptOptions popts( yn, default_answer ? 0 : 1 );
  if ( ! zypper.config().non_interactive )
    clear_keyboard_buffer();
  zypper.out().prompt( pid, question, popts );
  return !get_prompt_reply( zypper, pid, popts );
}

unsigned get_prompt_reply( Zypper & zypper, PromptId pid, const PromptOptions & poptions )
{
  // non-interactive mode: return the default reply
  if (zypper.config().non_interactive)
  {
    // print the reply for convenience (only for normal output)
    if ( ! zypper.config().machine_readable )
      zypper.out().info( poptions.options()[poptions.defaultOpt()],
			 Out::QUIET, Out::TYPE_NORMAL );
    MIL << "running non-interactively, returning " << poptions.options()[poptions.defaultOpt()] << endl;
    return poptions.defaultOpt();
  }

  // set runtimeData().waiting_for_input flag while in this function
  struct Bye
  {
    Bye( bool & flag ) : _flag( flag ) { _flag = true; }
    ~Bye() { _flag = false; }
    bool & _flag;
  } say_goodbye( zypper.runtimeData().waiting_for_input );

  // PENDING SigINT? Some frequently called place to avoid exiting from within the signal handler?
  zypper.immediateExitCheck();

  // open a terminal for input (bnc #436963)
  std::ifstream stm( "/dev/tty" );

  std::string reply;
  int reply_int = -1;
  while ( true )
  {
    reply = str::getline( stm, str::TRIM );
    // if we cannot read input or it is at EOF (bnc #436963), exit
    if ( ! stm.good() )
    {
      WAR << "Could not read the answer - bad stream or EOF" << endl;
      zypper.out().error( _("Cannot read input: bad stream or EOF."),
			  str::Format(_("If you run zypper without a terminal, use '%s' global\n"
			  "option to make zypper use default answers to prompts."))
			  % "--non-interactive" );
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
      ZYPP_THROW( ExitRequestException("Cannot read input. Bad stream or EOF.") );
    }

    // empty reply is a good reply (on enter)
    if ( reply.empty() )
    {
      reply_int = poptions.defaultOpt();
      break;
    }

    if ( reply == "?" )
    {
      zypper.out().promptHelp(poptions);
      continue;
    }

    if ( poptions.isYesNoPrompt() && rpmatch(reply.c_str()) >= 0 )
    {
      if ( rpmatch(reply.c_str()) )
        reply_int = 0; // the index of "yes" in the poptions.options()
      else
        reply_int = 1; // the index of "no" in the poptions.options()
      break;
    }
    else if ( (reply_int = poptions.getReplyIndex(reply)) >= 0 ) // got valid reply
      break;

    std::ostringstream s;
    s << str::Format(_("Invalid answer '%s'.")) % reply;
    if ( poptions.isYesNoPrompt() )
    {
      s << " "
        // translators: the %s are: 'y', 'yes' (translated), 'n', and 'no' (translated).
	<< str::Format(_("Enter '%s' for '%s' or '%s' for '%s' if nothing else works for you."))
	   % "y" % _("yes") % "n" % _("no");
    }
    zypper.out().prompt( pid, s.str(), poptions );
  }

  if ( reply.empty() )
    MIL << "reply empty, returning the default: "
        << poptions.options()[poptions.defaultOpt()] << " (" << reply_int << ")"
        << endl;
  else
    MIL << "reply: " << reply << " (" << reply_int << ")" << endl;

  return reply_int;
}

///////////////////////////////////////////////////////////////////
namespace
{
  static const char * prefill = nullptr;

  int init_line()
  {
    if ( prefill )
    {
      rl_replace_line( prefill, 1 );
      rl_end_of_line( 1, 0 );
      rl_redisplay();
    }
    return 1;
  }
} // namespace
///////////////////////////////////////////////////////////////////
std::string get_text( const std::string & prompt, const std::string & prefilled )
{
  std::string ret;

  /* Get a line from the user. */
  prefill = prefilled.c_str();
  rl_pre_input_hook = init_line;
  if ( char * line_read = ::readline( prompt.c_str() ) )
  {
    ret = line_read;
    ::free( line_read );
  }
  return ret;
}
