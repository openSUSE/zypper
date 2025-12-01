/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <cctype>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>

//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "Parser.h"

using std::cout;
using std::cerr;
using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace qf
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

#if WITH_TAGNAME_IDS
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
          reserved	%= qi::lit('%') | '[' | ']' | '}';

          string_value  %= +( unesc_char | '\\'>>qi::char_ | (qi::char_-reserved) );
          tok_string =
            string_value           [ px::bind(&String::value, _val) = _1 ];

          tag_fieldw	%= -qi::char_('-') >> *qi::char_('0') >> *qi::digit;
#if USE_TAGNAME_IDS
          tag_name      %= no_case[ attribute ];
#else
# if STORE_TAGNAME_LOWER
          tag_name	= identifyer [ _val = px::bind(&lowerstr,_1) ];
# else
          tag_name	%= identifyer;
# endif // STORE_TAGNAME_LOWER
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
      struct printer
      {
        typedef boost::spirit::utf8_string string;

        void element(string const& tag, string const& value, int depth) const
        {
          for (int i = 0; i < (depth*4); ++i) // indent to depth
            cerr << ' ';

          cerr << "tag: " << tag;
          if (value != "")
            cerr << ", value: " << value;
          cerr << std::endl;
        }
      };

      void print_info(boost::spirit::info const& what)
      {
        using boost::spirit::basic_info_walker;

        printer pr;
        basic_info_walker<printer> walker(pr, what.tag, 0);
        boost::apply_visitor(walker, what.value);
      }
    } // namespace

    bool parse( std::string_view qf_r, Format & result_r )
    {
      result_r.tokens.clear();

      using boost::spirit::qi::expectation_failure;
      using Iterator = std::string_view::iterator;

      parser::Grammar<Iterator> grammar;
      Iterator iter = qf_r.begin();
      Iterator end  = qf_r.end();

      bool res = false;
      try
      {
        res = parse( iter, end, grammar, result_r );
      }
      catch ( const expectation_failure<char const*> & x )
      {
        cerr << "Failed to parse: \"" << qf_r << '"' << endl;
        cerr << "     unexpected: \"" << std::string_view( x.first, x.last-x.first ) << '"' << std::endl;
        cerr << "       expected: "; print_info( x.what_ );
        return false;
      }

      if ( not res || iter != end ) {
        cerr << "Failed to parse: \"" << qf_r << '"' << endl;
        cerr << "     unexpected: \"" << std::string_view( iter, end-iter ) << '"' << std::endl;
        cerr << "       expected: literal special chars '%[]{}' need to be \\-escaped" << endl;
        return false;
      }

      return true;
    }

  } // namespace qf
} // namespace zypp
