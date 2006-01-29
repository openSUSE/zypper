/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/SelectionSelFileParser.cc
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

#include "zypp/source/susetags/SelectionSelFileParser.h"
#include <boost/regex.hpp>
#include "zypp/parser/tagfile/Grammar.h"

using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      static void dumpRegexpResults( const boost::smatch &what )
      {
        for ( unsigned int k=0; k < what.size(); k++)
        {
          DBG << "[match "<< k << "] [" << what[k] << "]" << std::endl; 
        }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void SelectionSelFileParser::parse( const Pathname & file_r, SelectionEntry &entry_r )
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        // read vendor
        while(!file.eof())
        {
          getline(file, buffer);
          boost::smatch what;
          if(boost::regex_match(buffer, what, boost::regex("^#(.*)$"), boost::match_extra))
          {
            DBG << "comment" << std::endl;
            // comment # something
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
          }
          else if(boost::regex_match(buffer, what, boost::regex("^\\+([a-zA-Z]+)(\\.([^[:space:]]+))?:$"), boost::match_extra))
          {
            DBG << "start list" << std::endl;
            dumpRegexpResults(what);
            // start of list +Something.lang:
            // lang is optional
            // str::strtonum(buffer, entry_r.count);
            std::set<std::string> elements;
            std::string element;
            boost::smatch element_what;
            getline(file, element);
            // while we dont find the list terminator
            while( ! boost::regex_match(element, element_what, boost::regex("^\\-([a-zA-Z]+)(\\.([_a-zA-Z]+))?:$"), boost::match_extra))
            {
              elements.insert(element);
              DBG << element << std::endl;
              getline(file, element);
              //dumpRegexpResults(element_what);
            }
            DBG << "end list" << std::endl;
            // end of list
          }
          else if(boost::regex_match(buffer, what, boost::regex("^=([a-zA-Z])+(\\.([^[:space:]]+))?:[\\s]*(.*)$"), boost::match_extra))
          {
            DBG << "assign" << std::endl;
            // start of list
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
          }
          else if(boost::regex_match(buffer, what, boost::regex("^([[:space:]]*)$"), boost::match_extra))
          {
            DBG << "empty line" << std::endl;
          }
          else
          {
            DBG << "parse error: " << buffer << std::endl;
          }
        }
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
