/*
IoBind Library License:
--------------------------

The zlib/libpng License Copyright (c) 2003 Jonathan de Halleux

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution
*/


#include <string>
#include <sstream>
#include <boost/format.hpp>
#include <boost/spirit.hpp> 
#include "parser_utils.hpp"
#include "xml_escape_parser.hpp"


namespace iobind{
namespace parser{
namespace detail{

struct escapes : boost::spirit::symbols<std::string>
{
    escapes()
    {
        add
            ("<"    , "lt")
            (">"    , "gt")
            ("&"    , "amp")
            ("'"    , "apos")
            ("\""    , "quot")
        ;
    }

} escapes_p;

struct unescapes : boost::spirit::symbols<std::string>
{
    unescapes()
    {
        add
            ("lt", "<")
            ("gt",">")
            ("amp","&")
            ("apos","\'")
            ("quot","\"")
        ;
    }
} unescapes_p;

struct unescape_from_xml_grammar : boost::spirit::grammar<unescape_from_xml_grammar>
{
	std::ostream& out;

	explicit unescape_from_xml_grammar( std::ostream& out_)
		:out(out_){};

   template <typename ScannerT>
   struct definition
   {    
        definition(unescape_from_xml_grammar const& self)  
		{
			using namespace boost::spirit;

			begin = ch_p('&');
			end = ch_p(';');
			// the rest is ok
			encoded_string=
				*( 
					 begin >> unescapes_p[concat_symbol(self.out)] >> end
				   | anychar_p[concat_string(self.out)]
				 );
		};

		boost::spirit::rule<ScannerT> encoded_string, begin, end;
		boost::spirit::rule<ScannerT> const& start() const { return encoded_string; };
   };
};

struct escape_to_xml_grammar : boost::spirit::grammar<escape_to_xml_grammar>
{
	std::ostream& out;

	explicit escape_to_xml_grammar( std::ostream& out_)
		:out(out_){};
  
   template <typename ScannerT>
   struct definition
   {    
        definition(escape_to_xml_grammar const& self)  
		{
			using namespace boost::spirit;
			concat_pre_post_symbol concatener(self.out, "&", ";");

			// the rest is ok
			encoded_string=
				*( 
					 escapes_p[concatener]
				   | anychar_p[concat_string(self.out)]
				 );
		};

		boost::spirit::rule<ScannerT> encoded_string;
		boost::spirit::rule<ScannerT> const& start() const { return encoded_string; };
   };
};

}; //details


std::string xml_escape_parser::escape( std::string const& str) const
{
	using namespace boost::spirit;

	std::ostringstream out;

	parse_info<> info = boost::spirit::parse(
               str.c_str(), 
			   detail::escape_to_xml_grammar(out)
			   );

	return out.str();
};

std::string xml_escape_parser::unescape( std::string const& str) const
{
	using namespace boost::spirit;

	std::ostringstream out;

	parse_info<> info = boost::spirit::parse(
               str.c_str(), 
               detail::unescape_from_xml_grammar(out)
			   );

	return out.str();
};
}; // parser
}; // iobind

