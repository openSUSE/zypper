/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <cctype>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "Parser.h"

///////////////////////////////////////////////////////////////////
namespace zypp::qf
{
  ///////////////////////////////////////////////////////////////////
  namespace parser
  {
    namespace px = boost::phoenix;
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    std::string lowerstr( std::string str_r )
    {
      std::transform( str_r.begin(), str_r.end(), str_r.begin(), [](unsigned char c){ return std::tolower(c); } );
      return std::move(str_r);
    }

    std::string upperstr( std::string str_r )
    {
      std::transform( str_r.begin(), str_r.end(), str_r.begin(), [](unsigned char c){ return std::toupper(c); } );
      return std::move(str_r);
    }

    /// Symbol table to unescape special chars
    struct unesc_char_ : qi::symbols<char, const char *>
    {
      unesc_char_()
      {
        add
        ( "\\a", "\a" ) // Produces a bell or similar alert.
        ( "\\b", "\b" ) // Backspaces one character.
        ( "\\f", "\f" ) // Outputs a form-feed character.
        ( "\\n", "\n" ) // Outputs a newline character sequence.
        ( "\\r", "\r" ) // Outputs a carriage return character.
        ( "\\t", "\t" ) // Causes a horizontal tab.
        ( "\\v", "\v" ) // Causes a vertical tab.
        ( "\\\\", "\\"  ) // Displays a backslash character.
        ;
      }
    } unesc_char;

#if USE_TAGNAME_IDS
    /// Sample symbol table to ci-parse attribute names to ids
    struct attributes : qi::symbols<char, unsigned>
    {
      attributes()
      {
        unsigned val = 0;
        add
        ( "name",     ++val )
        ( "version",  ++val )
        ( "release",  ++val )
        ( "edition",  ++val )
        ( "arch",     ++val )
        ( "size",     ++val )
        ;
      }
    } attribute;
#endif

    /// Queryformat grammar.
    template <typename Iterator>
    struct Grammar : qi::grammar<Iterator, Format()>
    {
      Grammar()
      : Grammar::base_type( format )
      {
        using px::ref;
        using qi::eps;
        using qi::_val;
        using qi::_1;
        using ascii::no_case;

        identifyer	%= +qi::alpha;
        reserved	%= qi::lit('%') | '[' | ']' | '{' | '}';

        string_value    %= +( unesc_char | '\\'>>qi::char_ | (qi::char_-reserved) );
        tok_string =
          string_value           [ px::bind(&String::value, _val) = _1 ];

        tag_fieldw	%= -qi::char_('-') >> +qi::digit;
#if USE_TAGNAME_IDS
        tag_name        %= no_case[ attribute ];
#else
# if STORE_TAGNAME_UPPER
        tag_name	= identifyer [ _val = px::bind(&upperstr,_1) ];
# else
        tag_name	%= identifyer;
# endif // STORE_TAGNAME_UPPER
#endif // USE_TAGNAME_IDS
        tag_format	%= identifyer;
        tok_tag =
          qi::lit('%')
          >> -tag_fieldw         [ px::bind(&Tag::fieldw, _val) = _1 ]/*[ px::bind(&Tag::setFormat, _val, _1 ) ]*/
          >> '{'
          > -qi::lit('=')        [ px::bind(&Tag::noarray, _val) = true ]
          > tag_name             [ px::bind(&Tag::name, _val)   = _1 ]
          > -(':'>>tag_format)   [ px::bind(&Tag::format, _val) = _1 ]
          > '}';

        tok_array =
          qi::lit('[')
          > format               [ px::bind(&Array::format, _val) = _1 ]
          > ']';

        tok_conditional =
          qi::lit("%|")
          > tag_name             [ px::bind(&Conditional::name, _val) = _1 ]
          > "?{"
          > format               [ px::bind(&Conditional::Tformat, _val) = _1 ]
          > '}'
            > -( ":{"
            > format             [ px::bind(&Conditional::Fformat, _val) = _1 ]
            > '}' )
          > '|';

        format =
            *(
              tok_string         [ _val += _1 ]
            | tok_tag            [ _val += _1 ]
            | tok_array          [ _val += _1 ]
            | tok_conditional    [ _val += _1 ]
            );

#define OUTS(N) N.name( #N )
        OUTS( identifyer );
        OUTS( reserved );
        OUTS( string_value );
        OUTS( tok_string );
        OUTS( tag_fieldw );
        OUTS( tag_name );
        OUTS( tag_format );
        OUTS( tok_tag );
        OUTS( tok_array );
        OUTS( tok_conditional );
        OUTS( format );
#undef OUTS
      }

      qi::rule<Iterator, std::string()>   identifyer;
      qi::rule<Iterator, std::string()>   reserved;

      qi::rule<Iterator, std::string()>   string_value;
      qi::rule<Iterator, String()>        tok_string;

      qi::rule<Iterator, std::string()>   tag_fieldw;
#if USE_TAGNAME_IDS
      qi::rule<Iterator, unsigned()>      tag_name;
#else
      qi::rule<Iterator, std::string()>   tag_name;
#endif
      qi::rule<Iterator, std::string()>   tag_format;
      qi::rule<Iterator, Tag()>           tok_tag;

      qi::rule<Iterator, Array()>         tok_array;
      qi::rule<Iterator, Conditional()>   tok_conditional;

      qi::rule<Iterator, Format()>        format;
    };
  } // namespace parser


  namespace {
    struct PrintOn
    {
      typedef boost::spirit::utf8_string string;

      PrintOn( std::ostream& str_r )
      : str{ str_r }
      {}

      void element( string const& tag, string const& value, int depth ) const
      {
        for ( int i = 0; i < (depth*4); ++i ) // indent to depth
          str << ' ';

        str << "tag: " << tag;
        if (value != "")
          str << ", value: " << value;
        str << std::endl;
      }

      std::ostream & str;
    };

    std::ostream& printInfoOn( std::ostream& str, boost::spirit::info const& what )
    {
      PrintOn pr { str };
      boost::spirit::basic_info_walker<PrintOn> walker( pr, what.tag, 0 );
      boost::apply_visitor( walker, what.value );

      return str;
    }
  } // namespace

  Format parse( std::string_view qf_r )
  {
    using boost::spirit::qi::expectation_failure;
    using Iterator = std::string_view::iterator;

    parser::Grammar<Iterator> grammar;
    Iterator iter = qf_r.begin();
    Iterator end  = qf_r.end();

    Format result;
    bool res = false;
    try
    {
      res = parse( iter, end, grammar, result );
    }
    catch ( const expectation_failure<char const*> & x )
    {
      std::ostringstream str;
      str << "Failed to parse: \"" << qf_r << '"' << endl;
      str << "     unexpected: \"" << std::string_view( x.first, x.last-x.first ) << '"' << std::endl;
      printInfoOn( str<< "       expected: ", x.what_ );
      throw( ParseException( str.str() ) );
    }

    if ( not res || iter != end ) {
      std::ostringstream str;
      str << "Failed to parse: \"" << qf_r << '"' << endl;
      str << "     unexpected: \"" << std::string_view( iter, end-iter ) << '"' << std::endl;
      str << "       expected: literal special chars '%[]{}' need to be \\-escaped" << endl;
      throw( ParseException( str.str() ) );
    }

    return result;
  }

  //////////////////////////////////////////////////////////////////
  namespace test
  {
    inline std::string asString( const Format & fmt_r )
    { std::ostringstream str; str << fmt_r; return str.str(); }

    unsigned parsetest( std::string_view qf_r, std::optional<std::string_view> expect_r )
    {
      std::string got;
      if ( expect_r && expect_r->empty() ) expect_r = qf_r;

      try {
        got = asString( parse( qf_r ) );
      }
      catch( const ParseException & ex ) {
        if ( expect_r ) {
          MOUT << "Failed to parse: \"" << qf_r << "\"" << endl;
          MOUT << "       expected: \"" << *expect_r << "\"" << endl;
          MOUT << "        but got: ParseException" << endl;
          return 1;
        }
        return 0;
      }

      if ( not expect_r ) {
        MOUT << "Failed to parse: \"" << qf_r << "\"" << endl;
        MOUT << "       expected: ParseException" << endl;
        MOUT << "        but got: \"" << got << "\"" << endl;
        return 1;
      }

      if ( got != expect_r ) {
        MOUT << "Failed to parse: \"" << qf_r << "\"" << endl;
        MOUT << "       expected: \"" << *expect_r << "\"" << endl;
        MOUT << "        but got: \"" << got << "\"" << endl;
        return 1;
      }

      return 0;
    }

    struct CharStr
    {
      CharStr( char c_r )
      : c      { c_r }
      , qf     { "_#_\\#_" }
      , expect { "_#_#_" }
      {
        qf[1] = expect[1] = qf[4] = expect[3] = c;

        switch( c ) {
          case '\\': // vanishes unless \-escaped
            expect = "__\\_";
            break;

          // translated into bell,backspace,...,CR if \-escaped
          case 'a': expect[3] = '\a'; break;
          case 'b': expect[3] = '\b'; break;
          case 't': expect[3] = '\t'; break;
          case 'n': expect[3] = '\n'; break;
          case 'v': expect[3] = '\v'; break;
          case 'f': expect[3] = '\f'; break;
          case 'r': expect[3] = '\r'; break;

          // literal special chars '%[]{}' need to be \-escaped
          case '%':
          case '[':
          case ']':
          case '{':
          case '}':
            qffail = qf;
            qf = "_\\#_";
            expect = "_#_";
            qf[2] = expect[1] = c;
            break;
        }
      }

      char c;
      std::string qf; ///< queryformat to parse \a c literal and/or \-escaped (not throwing)
      std::string expect; ///< expected parse result for qf
      std::optional<std::string> qffail; ///< if not empty: Queryformat throwing on literal special chars
    };

    inline unsigned parsetest( char c_r )
    {
      CharStr strs { c_r };
      unsigned fail = parsetest( strs.qf, strs.expect );
      if ( strs.qffail )
        fail += parsetest( *strs.qffail, std::nullopt );
      return fail;
    }

    unsigned parsetest()
    {
      unsigned fail = 0;

      for ( char c = 0; c >= 0; ++c ) {
        fail += parsetest( c );
      }
      fail += parsetest( "" );
      fail += parsetest( "***" );
      fail += parsetest( "***%-20{Tag:Format}***", "***%-20{TAG:Format}***" );
      fail += parsetest( "***[***%{=TAGA}***%{TAGB}***]***" );
      fail += parsetest( "***%|TAG?{***[***%{=TAGA}***%{TAGB}***]***}:{***%|TAG?{***[***%{=TAGA}***%{TAGB}***]***}:{***%-20{TAG:format}***}|***}|" );

      if ( fail )
        MOUT << "parsetest: " << fail << " failed!" << endl;
      else
        MOUT << "parsetest: OK" << endl;
      return fail;
    }

  } // namespace test
} // namespace zypp::qf
