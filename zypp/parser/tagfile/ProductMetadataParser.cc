/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/ProductMetadataParser.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/String.h"

#include "zypp/parser/tagfile/ProductMetadataParser.h"
#include <boost/regex.hpp>
#include "zypp/parser/tagfile/Grammar.h"

using namespace std;
using namespace boost;

typedef find_iterator<string::iterator> string_find_iterator;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void ProductMetadataParser::parse( const Pathname & file_r, ProductEntry &entry_r )
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        while(!file.eof())
        {
          getline(file, buffer);
          boost::regex e("^(([A-Z]+)(\\.([_A-Z0-9a-z]+)){0,1}) (.+)$");
          boost::smatch what;
          if(boost::regex_match(buffer, what, e, boost::match_extra))
          {
            if ( what.size() < 5 )
              std::cout << "ups!!!!" << std::endl;
            
            std::string key = what[2];
            std::string value = what[5];
            std::string modifier = what[4];
            if(key == "PRODUCT")
              entry_r.name = value;
            else if(key == "VERSION")
              entry_r.version = value;
            else if(key == "DISTPRODUCT")
              entry_r.dist = value;
            else if(key == "DISTVERSION")
              entry_r.dist_version = value;
             else if(key == "BASEPRODUCT")
              entry_r.base_product = value;
            else if(key == "BASEVERSION")
              entry_r.base_version = value;
            else if(key == "YOUTYPE")
              entry_r.you_type = value;
            else if(key == "YOUPATH")
              entry_r.you_path = value;
            else if(key == "YOUURL")
              entry_r.you_url = value;
            else if(key == "VENDOR")
              entry_r.vendor = value;
            else if(key == "RELNOTESURL")
              entry_r.release_notes_url = value;
            else if(key == "ARCH")
              parseLine( key, modifier, value, entry_r.arch);
            else
              DBG << "parse error" << std::endl;
          }
          else
          {
            std::cout << "** No Match found **\n";
          }
          //std::cout << "hola: [" << buffer << "]" << std::endl;
        }
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, list<string> > &container)
      {
        if( modif.size() == 0)
          str::split( value, std::back_inserter(container["default"]), " ");
        else
          str::split( value, std::back_inserter(container[modif]), " ");
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, string > &container)
      {
        if( modif.size() == 0)
          container["default"] = value;
        else
          container[modif] = value;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
