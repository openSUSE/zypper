/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/MediaMetadataParser.cc
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

#include "zypp/parser/tagfile/MediaMetadataParser.h"
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

      static void dumpRegexpResults( const boost::smatch &what )
      {
        for ( unsigned int k=0; k < what.size(); k++)
        {
          DBG << "[match "<< k << "] [" << what[k] << "]" << std::endl; 
        }
      }
      
      /*
        File:  media
        Location  /media.N/ directory on media
        Content  two or more lines of ASCII as follows
        <vendor>
        <YYYYMMDDHHMMSS>
        [<media count>]
        [<media flags>]
        [<media names>]

        Currently defined flags:
         
        doublesided 
        media is double sided, YaST will ask for 'front side' for odd-numbered media and 'back side' for even-numbered media.
        The default is single-sided media.

        <media names> may define alternate strings to use when asking to insert a certain media.
         They are defined as <key><whitespace><value> pairs, separated by \n.
         
      */
      
      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : Parser::parse
      //	METHOD TYPE : void
      //
      void MediaMetadataParser::parse( const Pathname & file_r, MediaEntry &entry_r )
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        // read vendor
        getline(file, entry_r.vendor);
        // read timestamp
        getline(file, entry_r.timestamp);
        // skip flags for now
        std::string media_count_str;
        
        getline(file, buffer);
        regex is_digit_rx("^[\\d]+$");
        boost::smatch what_digit;
        
        // for the first line here we dont have to consume one if
        // there was no media
        bool consume = false;  

        if(boost::regex_match(buffer, what_digit, is_digit_rx))
        {
          // it was the media count
          str::strtonum(buffer, entry_r.count);
          // consume another line
          consume = true;
        }
        
        while(!file.eof())
        {
          // probably is the first line after we dont find the media number
          if(consume)
            getline(file, buffer);
          // only once
          consume = true;
          boost::regex e("^MEDIA([\\d]+)(\\.([_A-Za-z]+)){0,1} (.+)$");
          boost::smatch what;
          if(boost::regex_match(buffer, what, e, boost::match_extra))
          {
            if ( what.size() < 5 )
              std::cout << "ups!!!!" << std::endl;
           
            dumpRegexpResults(what);
            
            std::string key = what[2];
            std::string value = what[5];
            std::string modifier = what[4];
          }
          else
          {
            DBG << "** No Match found:  " << buffer << std::endl;
          }
        }
      }

      void MediaMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, list<string> > &container)
      {
        if ( modif.size() == 0)
          parseLine( key, value, container["default"]); 
        else
          parseLine( key, value, container[modif]);
      }

      void MediaMetadataParser::parseLine( const string &key, const string &modif, const string &value, map< string, string > &container)
      {
        if( modif.size() == 0)
          container["default"] = value;
        else
          container[modif] = value;
      }

      void MediaMetadataParser::parseLine( const string &key, const string &value, std::list<std::string> &container)
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
