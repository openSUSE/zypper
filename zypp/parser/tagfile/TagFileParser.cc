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
#include "zypp/parser/tagfile/ParseException.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "TagFileParser"

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

      void dumpRegexpResults( const boost::smatch &what )
      {
        for ( unsigned int k=0; k < what.size(); k++)
        {
          XXX << "[match "<< k << "] [" << what[k] << "]" << std::endl;
        }
      }

      void dumpRegexpResults2( const boost::smatch &what )
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

        boost::regex rxComment("^[[:space:]]*#(.*)$");
        boost::regex rxMStart("^\\+([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$");
        boost::regex rxMEnd("^\\-([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$");
        boost::regex rxSStart("^=([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:[[:space:]]+(.*)$");
        boost::regex rxEmpty("^([[:space:]]*)$");

	if (!file) {
	    ZYPP_THROW (ParseException( "Can't open " + file_r.asString() ) );
	}

        std::string buffer;
        // read vendor
        MIL << "Started parsing " << file_r << std::endl;
        beginParse();
        while(file && !file.eof())
        {
          getline(file, buffer);
          boost::smatch what;
          if(boost::regex_match(buffer, what, rxComment, boost::match_extra))
          {
            XXX << "comment" << std::endl;
            // comment # something
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
          }
          else if(boost::regex_match(buffer, what, rxMStart, boost::match_extra))
          {
            MultiTag tag;
            tag.name = what[1];
            tag.modifier = what[3];

            XXX << "start list" << std::endl;
            dumpRegexpResults(what);
            // start of list +Something.lang:
            // lang is optional
            // str::strtonum(buffer, entry_r.count);
            std::string element;
            boost::smatch element_what;
            getline(file, element);
            // while we dont find the list terminator
            while( ! boost::regex_match(element, element_what, rxMEnd, boost::match_extra))
            {
              tag.values.push_back(element);
              XXX << element << std::endl;
              getline(file, element);
              //dumpRegexpResults(element_what);
            }
            XXX << "end list" << std::endl;
            consume(tag);
            // end of list
          }
          else if(boost::regex_match(buffer, what, rxSStart, boost::match_extra))
          {
            SingleTag tag;
            tag.name = what[1];
            tag.modifier = what[3];
            tag.value = what[4];
            XXX << "assign" << std::endl;
            // start of list
            // str::strtonum(buffer, entry_r.count);
            dumpRegexpResults(what);
            consume(tag);
          }
          else if(boost::regex_match(buffer, what, rxEmpty, boost::match_extra))
          {
            XXX << "empty line" << std::endl;
          }
          else
          {
            ERR << "parse error: " << buffer << std::endl;
          }
        }
        endParse();
        MIL << "Done parsing " << file_r << std::endl;
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
