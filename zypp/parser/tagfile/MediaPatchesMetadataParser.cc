/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/MediaPatchesMetadataParser.cc
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

#include "zypp/parser/tagfile/MediaPatchesMetadataParser.h"
#include <boost/regex.hpp>
#include "zypp/parser/tagfile/Grammar.h"

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
      void MediaPatchesMetadataParser::parse( const Pathname & file_r, MediaPatchesEntry &entry_r )
      {
        std::ifstream file(file_r.asString().c_str());
        std::string buffer;
        // read vendor
        getline(file, buffer);

        regex rx("^([\\S]+)( (.*))?$");
        boost::smatch what;

        if(boost::regex_match(buffer, what, rx))
        {
          dumpRegexpResults(what);

          // it was the media count
          //str::strtonum(buffer, entry_r.count);
          // consume another line
          //consume = true;
        }
        else
        {
          //entry_r.count = 1;
        }

        /* 
        while(!file.eof())
        {
          // probably is the first line after we dont find the media number
          if(consume)
            getline(file, buffer);
          
          // only skip once
          consume = true;
          boost::regex e("^MEDIA([\\d]+)(\\.([_A-Za-z]+)){0,1} (.+)$");
          boost::smatch what;
          if(boost::regex_match(buffer, what, e, boost::match_extra))
          {
            if ( what.size() < 5 )
              std::cout << "ups!!!!" << std::endl;
           
            dumpRegexpResults(what);
            
            unsigned int number = 1;
            str::strtonum( what[1], number);
            std::string lang = what[3];
            std::string desc = what[4];
            entry_r.alternate_names[number][lang] = desc;
          }
          else
          {
            DBG << "** No Match found:  " << buffer << std::endl;
          }
          
        }
        */
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
