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
#include "zypp/base/Exception.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/String.h"

#include "zypp/source/susetags/ProductMetadataParser.h"
#include <boost/regex.hpp>

using namespace std;
using namespace boost;

typedef find_iterator<string::iterator> string_find_iterator;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void ProductMetadataParser::parse( const Pathname & file_r, ProductEntry &entry_r )
      {
        std::ifstream file(file_r.asString().c_str());

	if (!file) {
	    ZYPP_THROW (Exception("Can't read product file :" + file_r.asString()));
	}

        std::string buffer;
        while(file && !file.eof())
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
            else if(key == "DEFAULTBASE")
              entry_r.default_base = value;
            else if(key == "REQUIRES")
              parseLine( key, value, entry_r.requires);
            else if(key == "LINGUAS")
              parseLine( key, value, entry_r.languages);
            else if(key == "LABEL")
              parseLine( key, modifier, value, entry_r.label);
            else if(key == "DESCRDIR")
              entry_r.description_dir = value;
            else if(key == "DATADIR")
              entry_r.data_dir = value;
            else if(key == "FLAGS")
              parseLine( key, value, entry_r.flags);
            else if(key == "LANGUAGE")
              entry_r.language = value;
            else if(key == "TIMEZONE")
              entry_r.timezone = value;
            else
              DBG << "parse error" << std::endl;
          }
          else
          {
            DBG << "** No Match found:  " << buffer << std::endl;
          }
        }
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, list<string> > &container)
      {
        if ( modif.size() == 0)
          parseLine( key, value, container["default"]); 
        else
          parseLine( key, value, container[modif]);
      }

      void ProductMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, string > &container)
      {
        if( modif.size() == 0)
          container["default"] = value;
        else
          container[modif] = value;
      }

      void ProductMetadataParser::parseLine( const string &key, const string &value, std::list<std::string> &container)
      {
          str::split( value, std::back_inserter(container), " ");
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
