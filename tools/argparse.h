#ifndef ZYPP_TOOLS_ARGPARSE_H
#define ZYPP_TOOLS_ARGPARSE_H

#include <iosfwd>
#include <string>
#include <exception>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//#include <regex> seems to be bad with gcc < 4.9
#include <zypp/base/Regex.h>

///////////////////////////////////////////////////////////////////
/// Simple arg parser for tools
/// \code
///  argparse::Options options;
///  options.add()
///    ( "help,h",	"Print help and exit." )
///    ( "root",	"Use the system located below ROOTDIR.", argparse::Option::Arg::required )
///    ;
///  auto result = options.parse( argc, argv );
///
///  if ( result.count( "root" ) )
///    sysRoot = result["root"].arg();
/// \endcode
namespace argparse
{
  using zypp::str::regex;
  using zypp::str::smatch;
  using zypp::str::regex_match;

  ///////////////////////////////////////////////////////////////////
  /// Exception thrown when defining Options or parsing.
  class OptionException : public std::exception
  {
  public:
    OptionException( std::string msg_r )
    : _msg { std::move(msg_r) }
    {}

    OptionException( std::string msg_r, const std::string & msg2_r )
    : _msg { std::move(msg_r) }
    { if ( ! msg2_r.empty() ) _msg += msg2_r; }

    const char* what() const noexcept override
    { return _msg.c_str(); }

  private:
    std::string _msg;
  };

  ///////////////////////////////////////////////////////////////////
  /// Option description (TBD define arg consumer)
  class Option
  {
  public:
    enum class Arg { none, required, optional };

  public:
    Option( std::string descr_r, Arg hasarg_r  )
    : _descr { std::move(descr_r) }
    , _hasarg { hasarg_r }
    {
      if ( hasarg_r == Arg::optional )
	throw OptionException( "Not yet implemented: Option::Arg::optional" );
    }

    const std::string & descr() const
    { return _descr; }

    Arg hasarg() const
    { return _hasarg; }

  private:
    std::string _descr;
    Arg         _hasarg;
  };


  class ParsedOptions;

  ///////////////////////////////////////////////////////////////////
  /// Map of option names -> option descriptions.
  class Options
  {
    typedef std::unordered_map<std::string, std::shared_ptr<const Option>> OptionMap;
  public:
    Options()
    {}

  public:
    class Injector
    {
    public:
      Injector( OptionMap & optmap_r )
      : _optmap { optmap_r }
      {}

      Injector & operator()( const std::string & names_r, std::string descr_r, Option::Arg hasarg_r = Option::Arg::none )
      {
	smatch result;
	if ( regex_match( names_r, result, regex("([[:alnum:]][-_[:alnum:]]+)(,([[:alnum:]]))?") ) )
	{
	  auto opt = std::make_shared<const Option>( std::move(descr_r), hasarg_r );
	  add( result[1], opt );
	  if ( ! result[3].empty() )
	    add( result[3], opt );
	}
	else
	  throw OptionException( "Illegal option names: ", names_r );

	return *this;
      }

    private:
      void add( std::string name_r, std::shared_ptr<const Option> opt_r )
      {
	if ( _optmap.count( name_r ) )
	  throw OptionException( "Duplicate option name: ", name_r );
	_optmap[name_r] = opt_r;
      }

    private:
      OptionMap & _optmap;
    };

    Injector add()
    { return Injector( _optmap ); }

  public:
    ParsedOptions parse( int argc, char * argv[] ) const;

  public:
    std::ostream & dumpOn( std::ostream & str_r ) const
    {
      str_r << "OPTIONS:";
      if ( ! _optmap.empty() )
      {
	std::unordered_map<std::shared_ptr<const Option>, std::string> unify;
	for ( const auto & p : _optmap )
	{
	  std::string & t { unify[p.second] };
	  if ( t.empty() )
	    t = (p.first.length()>1?"--":"-")+p.first;
	  else if ( p.first.length() > 1 )
	    t = "--"+p.first+", "+t;
	  else
	    t = t+", -"+p.first;
	}

	boost::format fmt( "\n    %1% %|30t|%2%" );
	for ( const auto & p : unify )
	{
	  fmt % p.second % p.first->descr();
	  str_r << fmt.str();
	}
      }
      else
      {
	str_r << "    This command accepts no options.";
      }
      return str_r;
    }

  private:
    OptionMap _optmap;
  };

  inline std::ostream & operator<<( std::ostream & str_r, const Options & obj_r )
  { return obj_r.dumpOn( str_r ); }

  ///////////////////////////////////////////////////////////////////
  /// Parsed option incl. option args value (by now just stores the string)
  class OptionValue
  {
  public:
    OptionValue( std::shared_ptr<const Option> opt_r )
    : _opt { opt_r }
    {}

    OptionValue( std::shared_ptr<const Option> opt_r, std::string arg_r )
    : _opt { opt_r }
    , _arg { std::make_shared<std::string>( std::move(arg_r) ) }
    {}

    const std::string & arg() const
    {
      if ( ! _arg )
	throw std::domain_error( "No arg value" );

      return *_arg;
    }

  private:
    std::shared_ptr<const Option> _opt;
    std::shared_ptr<std::string> _arg;
  };

  ///////////////////////////////////////////////////////////////////
  /// Parsed options and positional args
  class ParsedOptions
  {
    typedef std::unordered_map<std::string, std::shared_ptr<const Option>> OptionMap;
    typedef std::unordered_map<std::shared_ptr<const Option>, OptionValue> ResultMap;
  public:
    ParsedOptions( const OptionMap & optmap_r, int argc, char * argv[] )
    : _optmap { optmap_r }
    { parse( argc, argv ); }

  public:
    size_t count( const std::string & optname_r ) const
    {
      auto iter = _optmap.find( optname_r );
      if ( iter == _optmap.end() )
	return 0;

      auto resiter = _options.find( iter->second );
      return( resiter == _options.end() ? 0 : 1 );
    }

    const OptionValue & operator[]( const std::string & optname_r ) const
    {
      return requireOptValByName( optname_r );
    }

    const std::vector<std::string> & positionals() const
    { return _positionals; }

  private:
    void parse( int argc, char * argv[] )
    {
      bool collectpositional = false;
      for ( --argc,++argv; argc; --argc,++argv )
      {
	if ( (*argv)[0] == '-' && !collectpositional )
	{
	  if ( (*argv)[1] == '-' )
	  {
	    if ( (*argv)[2] == '\0' )
	    {
	      // -- rest are positional...
	      collectpositional = true;
	    }
	    else
	    {
	      // --longopt
	      parseoptl( (*argv)+2, argc, argv );
	    }
	  }
	  else
	  {
	    // -s(hortopt)
	    parseopts( (*argv)+1, argc, argv );
	  }
	}
	else
	{
	  // positional
	  _positionals.push_back( *argv );
	}
      }
    }

  private:
    std::string dashed( std::string name_r ) const
    { return name_r.insert( 0, name_r.size()>1?"--":"-" ); }

    void parseoptl( const std::string & name_r, int & argc, char **& argv )
    {
      if ( name_r.length() < 2 )
	throw OptionException( "Illegal long opt: --", name_r );

      parseopt( name_r, argc, argv );
    }

    void parseopts( const std::string & name_r, int & argc, char **& argv )
    {
     if ( name_r.length() != 1 )
	throw OptionException( "Illegal short opt: -", name_r );

     parseopt( name_r, argc, argv );
    }

    void parseopt( const std::string & name_r, int & argc, char **& argv )
    {
      auto opt = requireOptByName( name_r );

      auto iter = _options.find( opt );
      if ( iter != _options.end() )
	throw OptionException( "Multiple occurrences of option: ", dashed( name_r ) );

      if ( opt->hasarg() != Option::Arg::none )
      {
	if ( opt->hasarg() == Option::Arg::optional )
	  throw OptionException( "Not yet implemented: Option::Arg::optional" ); // i.e. '--opt=arg'

	if ( argc < 2 )
	  throw OptionException( "Missing argument for option: ", dashed( name_r ) );

	--argc,++argv;
	moveToResult( opt, OptionValue( opt, *argv ) );
      }
      else
	moveToResult( opt, OptionValue( opt ) );
    }

    void moveToResult( std::shared_ptr<const Option> opt_r, OptionValue && value_r )
    { _options.insert( std::make_pair( opt_r, std::move(value_r) ) ); }

    std::shared_ptr<const Option> requireOptByName( const std::string & name_r ) const
    {
      auto iter = _optmap.find( name_r );
      if ( iter == _optmap.end() )
	throw OptionException( "Unknown option: ", dashed( name_r ) );
      return iter->second;
    }

    const OptionValue & requireOptValByName( const std::string & name_r ) const
    {
      auto iter = _options.find( requireOptByName( name_r ) );
      if ( iter == _options.end() )
	throw OptionException( "Option not present: ", dashed( name_r ) );
      return iter->second;
    }

  private:
    const OptionMap &		_optmap;
    ResultMap			_options;
    std::vector<std::string>	_positionals;
  };

  inline ParsedOptions Options::parse( int argc, char * argv[] ) const
  { return ParsedOptions( _optmap, argc, argv ); }



} // namespace argparse
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TOOLS_ARGPARSE_H
