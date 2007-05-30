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
#include "zypp/PathInfo.h"

#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/parser/ParseException.h"


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

      TagFileParser::TagFileParser( ParserProgress::Ptr progress ) : _progress(progress)
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
        // save parsed filename for debug
        int previous_progress = 0;
        int new_progress = 0;
        _file_r = file_r;
        _file_size = 0;
        _line_number = 0;
        _file_size = PathInfo(file_r).size();
        std::ifstream file(file_r.asString().c_str());
        int readed = 0;
        
        boost::regex rxComment("^[[:space:]]*#(.*)$");
        boost::regex rxMStart("^\\+([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$");
        boost::regex rxMEnd("^\\-([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:$");
        boost::regex rxSStart("^=([^[:space:]^\\.]+)(\\.([^[:space:]]+))?:[[:space:]]*(.*)$");
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
          _line_number++;
          readed +=  buffer.size();
          
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
            _line_number++;
            readed +=  element.size();
            // while we dont find the list terminator
            while(!file.eof())
            {
              // avoid regexping in most cases.
              if ( element[0] == '-' )
              {
                if ( boost::regex_match(element, element_what, rxMEnd, boost::match_extra) )
                {
                  // end list element? we check that it is the same as the opening tag, otherwise it is all broken!
                  if ( tag.name != element_what[1] )
                    ZYPP_THROW(ParseException("Expecting tag -" + tag.name + " for closing. Found -" + element_what[1]));
                  
                  // no problem, is a real close list tag
                  break;
                }
              }
              
              // if we are in a multi tag (list), we cannot start a list inside a list, so if we find a
              // + sign, we check it. We dont just regexp every entry because it is very expensive
              if ( element[0] == '+' )
              {
                if ( boost::regex_match(element, element_what, rxMStart, boost::match_extra) )
                {
                  if ( tag.name != element_what[1] )
                    ZYPP_THROW(ParseException("MultiTag +" + element_what[1] + " started before closing +" + tag.name));
                  else
                    ZYPP_THROW(ParseException("MultiTag +" + tag.name + " duplicate opening tag"));
                }
              }
              
              tag.values.push_back(element);
              
              XXX << element << std::endl;
              getline(file, element);
              _line_number++;
              readed +=  element.size();
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
            // https://bugzilla.novell.com/show_bug.cgi?id=160607
            // before we used to throw a parse error exception if we dont find
            // a key value line. But package descriptions usually are broken
            // and contain multiple lines for single line tags, etc.
            stringstream ss;
            ss << "Parse error, unrecognized line [" << buffer << "]. Be sure " << _file_r << " line " << _line_number << " misses a tag or comment.";
            ZYPP_THROW( ParseException( ss.str() ) );
          }
          
          new_progress = (int)((((float)readed)/((float)_file_size))*100);
          if ( _progress && new_progress != previous_progress )
            _progress->progress( new_progress );
          previous_progress = new_progress;
          
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
