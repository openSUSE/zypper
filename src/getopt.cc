#include "getopt.h"
#include "main.h"
#include <iostream>
#include "zypp/base/String.h"

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

  while (1) {
    int option_index = 0;
    optc = getopt_long (argc, argv, optstring.c_str (),
			longopts, &option_index);
    if (optc == -1)
      break;			// options done

    switch (optc) {
    case '?':
      result["_unknown"].push_back("");
      cerr << _("Unknown option ") << argv[optind - 1] << endl;
      break;
    case ':':
      cerr << _("Missing argument for ") << argv[optind - 1] << endl;
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
