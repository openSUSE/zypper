#include "getopt.h"
#include "main.h"
#include <iostream>
#include <zypp/base/String.h>
#include "Zypper.h"

std::string longopts2optstring( const struct option* longopts )
{
  // + - do not permute, stop at the 1st nonoption, which is the command
  // : - return : to indicate missing arg, not ?
  std::string optstring( "+:" );
  for ( ; longopts && longopts->name; ++longopts )
  {
    if ( !longopts->flag && longopts->val )
    {
      optstring += (char)longopts->val;
      if ( longopts->has_arg == required_argument )
	optstring += ':';
      else if ( longopts->has_arg == optional_argument )
	optstring += "::";
    }
  }
  return optstring;
}

typedef std::map<int,const char*> short2long_t;

short2long_t make_short2long( const struct option* longopts )
{
  short2long_t result;
  for ( ; longopts && longopts->name; ++longopts )
  {
    if ( !longopts->flag && longopts->val )
    {
      // on the fly check for duplicate short args
      if ( ! result.insert( short2long_t::value_type( longopts->val, longopts->name ) ).second )
      {
	ZYPP_THROW(Exception(str::Str() << "duplicate short option -" << (char)longopts->val << " for --" << longopts->name << " and --" << result[longopts->val] ));
      }
    }
  }
  return result;
}

// longopts.flag must be NULL
parsed_opts parse_options( int argc, char** argv, const struct option* longopts )
{
  parsed_opts result;
  opterr = 0; 			// we report errors on our own
  int optc;
  std::string optstring( longopts2optstring( longopts ) );
  short2long_t short2long( make_short2long( longopts ) );

  // optind in the previous loop
  int optind_prev = optind;
  // short option position in a short option bunch argument ('-xyz')
  unsigned short_pos = 0;
  while ( true )
  {
    int option_index = 0;
    optc = getopt_long( argc, argv, optstring.c_str(), longopts, &option_index );
    if ( optc == -1 )
      break; // options done

    if ( optind == optind_prev )
      ++short_pos;

    switch ( optc )
    {
    case '?':
      // For '-garbage' argument, with 'a', 'b', and 'e' as known options,
      // getopt_long reports 'a', 'b', and 'e' as known options.
      // The rest ends here and it is either the last one from previous argument
      // (short_pos + 1 points to it), or the short_pos one from the current
      // argument. (bnc #299375)

      // wrong option in the last argument
      cerr << _("Unknown option ") << "'";

      if ( optind != optind_prev )
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
      result["_unknown"].push_back( "" );
      break;
    case ':':
      cerr << _("Missing argument for ") << argv[optind - 1] << endl;
      result["_missing_arg"].push_back( "" );
      break;
    default:
      const char *mapidx = optc? short2long[optc] : longopts[option_index].name;

      // creates if not there
      std::list<std::string>& value = result[mapidx];
      if ( optarg )
	value.push_back( optarg );
      else
	value.push_back( "" );
      break;
    }

    if ( optind != optind_prev )
      short_pos = 0;
    optind_prev = optind;
  }
  return result;
}

TriBool get_boolean_option( Zypper & zypper, const std::string & pname, const std::string & nname )
{
  // translators: speaking of two mutually contradicting command line options
  static std::string msg_contradition = _("%s used together with %s, which contradict each other. This property will be left unchanged.");

  TriBool result = indeterminate;
  if ( copts.count(pname) )
    result = true;
  if ( copts.count(nname) )
  {
    if ( result )
    {
      std::string po( "--" + pname );
      std::string no( "--" + nname );
      // report contradition
      zypper.out().warning(str::form( msg_contradition.c_str(), po.c_str(), no.c_str()), Out::QUIET );
      result = indeterminate;
    }
    else
      result = false;
  }
  return result;
}

Args::Args( const std::string & s )
: _argv( NULL )
{
  OIter oit( this );
  str::splitEscaped( s, oit );
}

void Args::clear_argv()
{
  if ( _argv != NULL )
  {
    for ( char **pp = _argv; *pp != NULL; ++pp )
      free( *pp );
    delete[] _argv;
    _argv = NULL;
  }
}

void Args::make_argv()
{
  int c = _args.size();
  _argv = new char* [c + 1];
  char **pp = _argv;
  for ( int i = 0; i < c; ++i )
  {
    *pp++ = strdup( _args[i].c_str() );
  }
  *pp = NULL;
}
// Local Variables:
// c-basic-offset: 2
// End:
