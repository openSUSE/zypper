#include "getopt.h"
#include "main.h"
#include <iostream>
#include "zypp/base/String.h"
#include "Zypper.h"

using namespace std;

string longopts2optstring (const struct option *longopts) {
  // + - do not permute, stop at the 1st nonoption, which is the command
  // : - return : to indicate missing arg, not ?
  string optstring = "+:";
  for (; longopts && longopts->name; ++longopts) {
    if (!longopts->flag && longopts->val) {
      optstring += (char) longopts->val;
      if (longopts->has_arg == required_argument)
	optstring += ':';
      else if (longopts->has_arg == optional_argument)
	optstring += "::";
    }
  }
  //cerr << optstring << endl;
  return optstring;
}

typedef map<int,const char *> short2long_t;

short2long_t make_short2long (const struct option *longopts) {
  short2long_t result;
  for (; longopts && longopts->name; ++longopts) {
    if (!longopts->flag && longopts->val) {
      result[longopts->val] = longopts->name;
    }
  }
  return result;
}

// longopts.flag must be NULL
parsed_opts parse_options (int argc, char **argv,
			   const struct option *longopts) {
  parsed_opts result;
  opterr = 0; 			// we report errors on our own
  int optc;
  string optstring = longopts2optstring (longopts);
  short2long_t short2long = make_short2long (longopts);

  // optind in the previous loop
  int optind_prev = optind;
  // short option position in a short option bunch argument ('-xyz')
  unsigned short_pos = 0;
  while (1) {
    int option_index = 0;
    optc = getopt_long(argc, argv, optstring.c_str (), longopts, &option_index);
    if (optc == -1)
      break; // options done

    if (optind == optind_prev)
      ++short_pos;

    switch (optc) {
    case '?':
      // For '-garbage' argument, with 'a', 'b', and 'e' as known options,
      // getopt_long reports 'a', 'b', and 'e' as known options.
      // The rest ends here and it is either the last one from previous argument
      // (short_pos + 1 points to it), or the short_pos one from the current
      // argument. (bnc #299375)

      // wrong option in the last argument
      cerr << _("Unknown option ") << "'";

      if (optind != optind_prev)
      {
        // last argument was a short option bunch, report only the last one
        if (short_pos)
          cerr << *(argv[optind - 1] + short_pos+1);
        else
          cerr << argv[optind - 1];
      }
      // wrong option in current argument, which is a short option bunch
      else
        cerr << *(argv[optind] + short_pos);

      cerr << "'" << endl;

      // tell the caller there have been unknown options encountered
      result["_unknown"].push_back("");
      break;
    case ':':
      cerr << _("Missing argument for ") << argv[optind - 1] << endl;
      result["_missing_arg"].push_back("");
      break;
    default:
      const char *mapidx = optc? short2long[optc] : longopts[option_index].name;

      // creates if not there
      list<string>& value = result[mapidx];
      if (optarg)
	value.push_back (optarg);
      else
	value.push_back ("");
      break;
    }

    if (optind != optind_prev)
      short_pos = 0;
    optind_prev = optind;
  }
  return result;
}

using boost::tribool;
using boost::indeterminate;

tribool get_boolean_option(
    Zypper & zypper,
    const string & pname,
    const string & nname )
{
  static string msg_contradition =
    // translators: speaking of two mutually contradicting command line options
    _("%s used together with %s, which contradict each other."
      " This property will be left unchanged.");

  tribool result = indeterminate;
  if (copts.count(pname))
    result = true;
  if (copts.count(nname))
  {
    if (result)
    {
      string po = "--" + pname;
      string no = "--" + nname;
      // report contradition
      zypper.out().warning(zypp::str::form(
          msg_contradition.c_str(), po.c_str(), no.c_str()), Out::QUIET);

      result = indeterminate;
    }
    else
      result = false;
  }
  return result;
}

Args::Args (const std::string& s)
  : _argv(NULL) {
  OIter oit (this);
  zypp::str::splitEscaped (s, oit);
}

void Args::clear_argv () {
  if (_argv != NULL) {
    for (char **pp = _argv; *pp != NULL; ++pp)
      free (*pp);
    delete[] _argv;
    _argv = NULL;
  }
}
void Args::make_argv () {
  int c = _args.size ();
  _argv = new char* [c + 1];
  char **pp = _argv;
  for (int i = 0; i < c; ++i) {
    *pp++ = strdup (_args[i].c_str());
  }
  *pp = NULL;
}
// Local Variables:
// c-basic-offset: 2
// End:
