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

//! \todo The default values seems to be useless - we always want to auto-return 'retry' in case of no user input.
//!
//! README first: This function seems to be part of media.cc, it expects Retry being given as default, this also explains why it
//! unconditionally writes out strings like "trying again".
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
    while ( const auto pollRes = poll( &pollfds, 1, 5 ) ) // some user input, timeout 5msec
    {
      if ( pollRes == -1 && errno == EINTR ) {
        // check for sigint
        zypper.immediateExitCheck();
        continue;
      }

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

    // check for sigint
    zypper.immediateExitCheck();

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
  zypper.out().prompt( pid, _("Abort, retry, ignore?"), popts );
  return get_prompt_reply( zypper, pid, popts );
}

// ----------------------------------------------------------------------------

bool read_bool_answer( PromptId pid, const std::string & question, bool default_answer )
{
  Zypper & zypper( Zypper::instance() );
  std::string yn( std::string(_("yes")) + "/" + _("no") );
  PromptOptions popts( yn, default_answer ? 0 : 1 );
  zypper.out().prompt( pid, question, popts );
  return !get_prompt_reply( zypper, pid, popts );
}

std::pair<bool,bool> read_bool_answer_opt_save( PromptId pid_r, const std::string & question_r, bool defaultAnswer_r )
{
  Zypper & zypper( Zypper::instance() );
  PromptOptions popts( { _("yes"), _("no"), _("always"), _("never") }, defaultAnswer_r ? 0 : 1 );

  zypper.out().prompt( pid_r, question_r, popts );
  unsigned r = get_prompt_reply( zypper, pid_r, popts );
  return { !(r%2), (r > 1) };
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

  clear_keyboard_buffer();

  // PENDING SigINT? Some frequently called place to avoid exiting from within the signal handler?
  zypper.immediateExitCheck();

  const auto &bailOut = [&](){
    WAR << "Could not read the answer - bad stream or EOF" << endl;
    zypper.out().error( _("Cannot read input: bad stream or EOF."),
                        str::Format(_("If you run zypper without a terminal, use '%s' global\n"
                        "option to make zypper use default answers to prompts."))
                        % "--non-interactive" );
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    ZYPP_THROW( ExitRequestException("Cannot read input. Bad stream or EOF.") );
  };

  // open a terminal for input (bnc #436963)
  zypp::AutoFILE stm( fopen("/dev/tty", "r") );
  if (!stm) bailOut();

  std::string reply;
  int reply_int = -1;
  unsigned loopcnt = 0;
  while ( /*true*/++loopcnt ) {
    pollfd pollfds;
    pollfds.fd = fileno(stm);
    pollfds.events = POLLIN; // wait only for data to read

    while ( true )
    {
      //! Since we are operating on a tty, which is line based, poll will only
      //! return if there is a line to read in the buffer.
      //! We can use that behavior with a timeout to check for the ctrl+c flag
      const auto pollRes = poll( &pollfds, 1, 1 * 1000 ); // wait for user input, timeout 1 sec
      if ( pollRes == -1 ) {
        if ( errno == EINTR ) {
          // check for sigint
          zypper.immediateExitCheck();
          continue;
        }
        else bailOut();
      }

      // timeout, check for SIGINT, then continue to wait for input
      if ( pollRes == 0 ) {
        zypper.immediateExitCheck();
        continue;
      }

      zypp::AutoFREE<char> rawLine((char *)nullptr);
      size_t len = 0;

      ssize_t nread = ::getline( &rawLine.value(), &len, stm );
      // if we cannot read input or it is at EOF (bnc #436963), exit
      if ( nread == -1 ) bailOut();

      //getline returns the \n so we need to remove that
      if ( nread == 1 ) {
        reply = "";
      } else {
        reply = str::trim( std::string( rawLine.value(), 0, nread - 1) );
      }

      break;
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

    if ( poptions.isYesNoPrompt() && ::rpmatch(reply.c_str()) >= 0 )
    {
      if ( ::rpmatch(reply.c_str()) )
        reply_int = 0; // the index of "yes" in the poptions.options()
      else
        reply_int = 1; // the index of "no" in the poptions.options()
      break;
    }

    std::vector<int> replyMatches { poptions.getReplyMatches( reply ) };

    if ( replyMatches.size() == 1 )	// got valid reply
    {
      reply_int = replyMatches[0];
      break;
    }

    ColorStream s { ColorContext::MSG_ERROR };
    if ( replyMatches.empty() )
      s << ": " << str::Format(_("Invalid answer '%s'.")) % reply;
    else
    {
      s << ": " << str::Format(_("Ambiguous answer '%s'.")) % reply;
      s << " " << poptions.replyMatchesStr( replyMatches );
    }

    if ( poptions.isYesNoPrompt() )	// legacy message for YesNo as it uses ::rpmatch
    {
      s << " "
        // translators: the %s are: 'y', 'yes' (translated), 'n', and 'no' (translated).
        << str::Format(_("Enter '%s' for '%s' or '%s' for '%s' if nothing else works for you."))
           % "y" % _("yes") % "n" % _("no");
    }
    else if ( loopcnt > 1 )
    {
      s << " " << _("If nothing else works enter '#1' to select the 1st option, '#2' for the 2nd one, ...");
    }
    zypper.out().prompt( pid, s.str()+="\n", poptions );	// trailing uncolored '\n' is important, otherwise prompt inserts a leading ' '
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
