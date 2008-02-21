#include <ctype.h>
#include <sstream>

#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Deprecated.h"
#include "zypp/Resolvable.h"

#include "AliveCursor.h"
#include "zypper.h"
#include "zypper-main.h"
#include "output/Out.h"

using namespace std;
using namespace boost;

// ----------------------------------------------------------------------------

//template<typename Action>
//Action ...
int read_action_ari (PromptId pid, int default_action) {
  Out & out = Zypper::instance()->out();
  out.prompt(pid, _("Abort, retry, ignore?"), "[a/r/i]"); //! \todo translation?

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
  while (cin.good()) {		// #269263
    char c;
    cin >> c;
    c = tolower (c);
    MIL << "answer: " << c << endl;
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    // translators: don't translate the letters
    out.prompt(pid,
     boost::str(format(_("Invalid answer '%s'.")) % c),
      _("Choose letter 'a', 'r', or 'i'"));
    DBG << "invalid answer" << endl;
  }

  return default_action;
}

// ----------------------------------------------------------------------------

bool read_bool_answer(PromptId pid, const string & question, bool default_answer)
{
  const GlobalOptions & gopts = Zypper::instance()->globalOpts();
  Out & out = Zypper::instance()->out();

  ostringstream s;
  s << "[" << _("yes") << "/" << _("no") << "]";
  out.prompt(pid, question, s.str());

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

  string c = "";
  bool been_here_before = false;
  while (stm.good() && rpmatch(c.c_str()) == -1)
  {
    if (been_here_before)
      out.prompt(pid,
          boost::str(format(_("Invalid answer '%s'.")) % c),
          boost::str(format(
          // TranslatorExplanation don't translate the 'y' and 'n', they can always be used as answers.
          // The second and the third %s is the translated 'yes' and 'no' string (lowercase).
          _("Enter 'y' for '%s' or 'n' for '%s' if nothing else works for you"))
          % _("yes") % _("no")));
    c = zypp::str::getline (stm, zypp::str::TRIM);
    been_here_before = true;
  }

  MIL << "answer: " << c << endl;
  int answer = rpmatch(c.c_str());
  if (answer >= 0)
    return answer;
  else // in case of !stm.good()
  {
    WAR << "could not read answer, returning default: "
        << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }
}

// ----------------------------------------------------------------------------

string to_string (zypp::Resolvable::constPtr resolvable) {
  ostringstream ss;
  ss << *resolvable;
  return ss.str ();
}

// ----------------------------------------------------------------------------

ZYPP_DEPRECATED void report_zypp_exception(const zypp::Exception & e)
{
  if (e.historySize())
  {
    if (Zypper::instance()->globalOpts().verbosity > VERBOSITY_NORMAL)
    {
      // print the whole history
      cerr << e.historyAsString();
      // this exception
      cerr << " - " << e.asUserString();
    }
    else
      // print the root cause only
      cerr << *(--e.historyEnd());
  }
  else
    cerr << e.asUserString();
  cerr << endl;
}

// ----------------------------------------------------------------------------

ZYPP_DEPRECATED void report_problem(const zypp::Exception & e,
                    const string & problem_desc,
                    const string & hint)
{
  // problem
  cerr << problem_desc << endl;

  // cause
  report_zypp_exception(e);

  // hint
  if (!hint.empty())
    cerr << hint << endl;
}

// ----------------------------------------------------------------------------

void report_too_many_arguments(const string & specific_help)
{
  //! \todo make this more explanatory, e.g. "Ingoring arg1 arg2. This command does not take arguments. See %s for more information."
  ostringstream s;
  s << _("Usage") << ':' << endl << specific_help;
  Zypper::instance()->out().error(_("Too many arguments."), s.str());
}

// Local Variables:
// c-basic-offset: 2
// End:
