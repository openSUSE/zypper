/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/TagFileParser.cc
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


#include "zypp/parser/tagfile/TagFileParser.h"
#include <boost/regex.hpp>


using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      static void dumpRegexpResults( const boost::smatch &what )
      {
        for ( unsigned int k=0; k < what.size(); k++)
        {
          DBG << "[match "<< k << "] [" << what[k] << "]" << std::endl; 
        }
      }

      TagFileParser::TagFileParser()
      {
      }

      void TagFileParser::beginParse()
      {
      }

      void TagFileParser::endParse()
      {
      }

      void TagFileParser::consume( const SingleTag &tag )
      {
      }
      
      void TagFileParser::consume( const MultiTag &tag )
      {
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void TagFileParser::parse( const Pathname & file_r)
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        // read vendor
        beginParse();
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
          else if(boost::regex_match(buffer, what, boost::regex("^\\+([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$"), boost::match_extra))
          {
            MultiTag tag;
            tag.name = what[1];
            tag.modifier = what[3];

            DBG << "start list" << std::endl;
            dumpRegexpResults(what);
            // start of list +Something.lang:
            // lang is optional
            // str::strtonum(buffer, entry_r.count);
            std::string element;
            boost::smatch element_what;
            getline(file, element);
            // while we dont find the list terminator
            while( ! boost::regex_match(element, element_what, boost::regex("^\\-([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$"), boost::match_extra))
            {
              tag.values.insert(element);
              DBG << element << std::endl;
              getline(file, element);
              //dumpRegexpResults(element_what);
            }
            DBG << "end list" << std::endl;
            consume(tag);
            // end of list
          }
          else if(boost::regex_match(buffer, what, boost::regex("^=([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:[[:space:]]+(.*)$"), boost::match_extra))
          {
            SingleTag tag;
            tag.name = what[1];
            tag.modifier = what[3];
            tag.value = what[4];
            DBG << "assign" << std::endl;
            // start of list
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
            consume(tag);
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
        endParse();
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
