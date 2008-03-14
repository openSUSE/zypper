#include <ctype.h>
#include <sstream>

#include <boost/format.hpp>

#include "zypp/base/Logger.h"

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

// ----------------------------------------------------------------------------

//template<typename Action>
//Action ...
int read_action_ari (PromptId pid, int default_action) {
  Out & out = Zypper::instance()->out();
  // translators: "a/r/i" are the answers to the
  // "Abort, retry, ignore?" prompt
  // Translate the letters to whatever is suitable for your language.
  // the anserws must be separated by slash characters '/' and must
  // correspond to abort/retry/ignore in that order.
  // The answers should be lower case letters.
  PromptOptions popts(_("a/r/i"), 0);
  out.prompt(pid, _("Abort, retry, ignore?"), popts);

  // choose abort if no default has been specified
  if (default_action == -1) {
    default_action = 0;
  }

  // non-interactive mode
  if (Zypper::instance()->globalOpts().non_interactive) {
      char c;
      switch (default_action) {
          case 0: c = 'a'; break;
          case 1: c = 'r'; break;
          case 2: c = 'i'; break;
          default: c = '?';
      }
      // print the answer for conveniecne (only for normal output)
      out.info(string(1, c), Out::QUIET, Out::TYPE_NORMAL);
      MIL << "answer: " << c << endl;
      return default_action;
  }

  // interactive mode, ask user
  while (cin.good()) {          // #269263
    char c;
    cin >> c;
    MIL << "answer: " << c << endl;
    c = tolower (c);
    MIL << "answer: " << c << endl;
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    else if (c == 0)
      return default_action;

    // translators: don't translate the letters
    ostringstream s;
    s << format(_("Invalid answer '%s'.")) % c << " "
      << _("Choose letter 'a', 'r', or 'i'"); 
    out.prompt(pid, s.str(), popts); //! \todo remove this, handle invalid answers within the first prompt()
    DBG << "invalid answer" << endl;
  }

  return default_action;
}

// ----------------------------------------------------------------------------

bool read_bool_answer(PromptId pid, const string & question, bool default_answer)
{
  const GlobalOptions & gopts = Zypper::instance()->globalOpts();
  Out & out = Zypper::instance()->out();

  string yn = string(_("yes")) + "/" + _("no");

  PromptOptions popts(yn, default_answer ? 0 : 1);
  out.prompt(pid, question, popts);

  // non-interactive mode: print the answer for convenience  (only for normal
  // output) and return default
  if (gopts.non_interactive)
  {
    if (!gopts.machine_readable)
      out.info((default_answer ? _("yes") : _("no")), Out::QUIET, Out::TYPE_NORMAL);
    MIL << "answer (default): " << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }

  istream & stm = cin;

  string c;
  bool been_here_before = false;
  while (stm.good())
  {
    if (been_here_before)
    {
      ostringstream s;
      s << format(_("Invalid answer '%s'.")) % c << " " << format(
        // TranslatorExplanation don't translate the 'y' and 'n', they can always be used as answers.
        // The second and the third %s is the translated 'yes' and 'no' string (lowercase).
        _("Enter 'y' for '%s' or 'n' for '%s' if nothing else works for you"))
        % _("yes") % _("no");
      out.prompt(pid, s.str(), popts); //! \todo remove this, handle invalid answers within the first prompt()
    }
    c = zypp::str::getline (stm, zypp::str::TRIM);
    if (rpmatch(c.c_str()) >= 0 || c.empty())
      break;
    been_here_before = true;
  }

  MIL << "answer: " << c << endl;
  int answer = c.empty() ?  default_answer : rpmatch(c.c_str());
  if (answer >= 0)
    return answer;
  else // in case of !stm.good()
  {
    WAR << "could not read answer, returning default: "
        << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }
}
