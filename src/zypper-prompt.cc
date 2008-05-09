#include <ctype.h>
#include <iostream>
#include <sstream>
#include <poll.h>

#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypper.h"
#include "zypper-prompt.h"

using namespace std;
using namespace boost;

// ----------------------------------------------------------------------------

PromptOptions::PromptOptions(const std::string & option_str, unsigned int default_opt)
{
  setOptions(option_str, default_opt);
}

// ----------------------------------------------------------------------------

PromptOptions::~PromptOptions()
{

}

// ----------------------------------------------------------------------------

void PromptOptions::setOptions(const std::string & option_str, unsigned int default_opt)
{
  zypp::str::split(option_str, back_inserter(_options), "/"); 

  if (_options.size() <= default_opt)
    INT << "Invalid default option index " << default_opt << endl;
  else
    _default = default_opt; 
}

const string PromptOptions::optionString() const
{
  string option_str;
  StrVector::const_iterator it;
  if ((it = options().begin()) != options().end())
  {
    option_str += (defaultOpt() == 0 ? zypp::str::toUpper(*it) : *it);
    ++it;
  }
  for (unsigned int i = 1; it != options().end(); ++it, i++)
    option_str += "/" + (defaultOpt() == i ? zypp::str::toUpper(*it) : *it);
  
  if (!_opt_help.empty())
    option_str += "/?";

  return option_str;
}

void PromptOptions::setOptionHelp(unsigned int opt, const std::string & help_str)
{
  if (help_str.empty())
    return;

  if (opt >= _options.size())
  {
    WAR << "attempt to set option help for non-existing option."
        << " text: " << help_str << endl;
    return;
  }

  if (opt >= _opt_help.capacity())
    _opt_help.reserve(_options.size());
  if (opt >= _opt_help.size())
    _opt_help.resize(_options.size());

  _opt_help[opt] = help_str;
}

// ----------------------------------------------------------------------------

const std::string ari_mapping[] = { string(_("abort")),string(_("retry")),string(_("ignore"))};

int read_action_ari_with_timeout (PromptId pid, unsigned timeout,
    int default_action) {
  Out & out = Zypper::instance()->out();

  if (default_action >2 || default_action < 0)
  {
    WAR << "bad default action" << endl;
    default_action = 0;
  }

  out.info (_("Abort, retry, ignore?\n"));
  
  //FIXME XML output
  cout << endl;

  while (timeout)
  {
    char c = 0;
    pollfd pollfds;
    pollfds.fd = 0; //stdin
    pollfds.events = POLLIN; //wait only for data to read

    while (poll(&pollfds,1,5)){ //some user input, timeout 5msec
      c = getchar();
#define eat_rest_input() do {} while (getchar()!='\n')
      switch (c){
        case 'a':
        case 'A':
          eat_rest_input();
          return 0;
        case 'r':
        case 'R':
          eat_rest_input();
          return 1;
        case 'i':
        case 'I':
          eat_rest_input();
          return 2;
        default:
        WAR << "Unknown char " << c << endl;
      }
    }

    //FIXME XML output
    cout << "\r";
    cout << boost::str(format(_("autoselect %s after %u ")) % ari_mapping[default_action]
      % timeout); //! \todo fix english after 11.0
    cout.flush();

    sleep(1);
    timeout--;
  }
  
  return default_action;
}


// ----------------------------------------------------------------------------
//template<typename Action>
//Action ...
int read_action_ari (PromptId pid, int default_action)
{
  Zypper & zypper = *Zypper::instance();
  // translators: "a/r/i" are the answers to the
  // "Abort, retry, ignore?" prompt
  // Translate the letters to whatever is suitable for your language.
  // the answers must be separated by slash characters '/' and must
  // correspond to abort/retry/ignore in that order.
  // The answers should be lower case letters.
  PromptOptions popts(_("a/r/i"), (unsigned int) default_action);
  zypper.out().prompt(pid, _("Abort, retry, ignore?"), popts);
  return get_prompt_reply(zypper, pid, popts);
}

// ----------------------------------------------------------------------------

bool read_bool_answer(PromptId pid, const string & question, bool default_answer)
{
  Zypper & zypper = *Zypper::instance();
  string yn = string(_("yes")) + "/" + _("no");
  PromptOptions popts(yn, default_answer ? 0 : 1);
  zypper.out().prompt(pid, question, popts);
  return !get_prompt_reply(zypper, pid, popts);
}

unsigned int get_prompt_reply(Zypper & zypper,
                              PromptId pid,
                              const PromptOptions & poptions)
{
  // non-interactive mode: return the default reply
  if (zypper.globalOpts().non_interactive)
  {
    // print the reply for convenience (only for normal output)
    if (!zypper.globalOpts().machine_readable)
      zypper.out().info(poptions.options()[poptions.defaultOpt()],
        Out::QUIET, Out::TYPE_NORMAL);
    MIL << "running non-interactively, returning "
        << poptions.options()[poptions.defaultOpt()] << endl;
    return poptions.defaultOpt();
  }

  istream & stm = cin;
  bool is_yn_prompt =
    poptions.options().size() == 2 &&
    poptions.options()[0] == _("yes") &&
    poptions.options()[1] == _("no");

  string reply;
  unsigned int reply_int = poptions.defaultOpt();
  bool stmgood;
  while ((stmgood = stm.good()))
  {
    reply = zypp::str::getline (stm, zypp::str::TRIM);

    // empty reply is a good reply (on enter)
    if (reply.empty())
      break;

    if (reply == "?")
    {
      zypper.out().promptHelp(poptions);
      continue;
    }

    if (is_yn_prompt && rpmatch(reply.c_str()) >= 0)
    {
      if (rpmatch(reply.c_str()))
        reply_int = 0; // the index of "yes" in the poptions.options()
      else
        reply_int = 1; // the index of "no" in the poptions.options()
      break;
    }
    else
    {
      DBG << " reply: " << reply << " (" << zypp::str::toLower(reply) << " lowercase)" << endl;
      bool got_valid_reply = false;
      for (unsigned int i = 0; i < poptions.options().size(); i++)
      {
        DBG << "index: " << i << " option: "
            << poptions.options()[i] << endl; 
        if (poptions.options()[i] == zypp::str::toLower(reply))
        {
          reply_int = i;
          got_valid_reply = true;
          break;
        }
      }
      if (got_valid_reply)
        break;
    }

    ostringstream s;
    s << format(_("Invalid answer '%s'.")) % reply;

    if (is_yn_prompt)
    {
      s << " " << format(
      // TranslatorExplanation don't translate the 'y' and 'n', they can always be used as answers.
      // The second and the third %s is the translated 'yes' and 'no' string (lowercase).
      _("Enter 'y' for '%s' or 'n' for '%s' if nothing else works for you."))
      % _("yes") % _("no");
    }

    zypper.out().prompt(pid, s.str(), poptions);
  }

  if (!stmgood)
  {
    WAR << "Could not read the answer, returning the default: "
        << poptions.options()[poptions.defaultOpt()] << " (" << reply_int << ")"
        << endl;
    return poptions.defaultOpt();
  }

  if (reply.empty())
    MIL << "reply empty, returning the default: "
        << poptions.options()[poptions.defaultOpt()] << " (" << reply_int << ")"
        << endl; 
  else
    MIL << "reply: " << reply << " (" << reply_int << ")" << endl;

  return reply_int;
}
