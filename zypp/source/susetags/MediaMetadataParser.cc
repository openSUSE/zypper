/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/MediaMetadataParser.cc
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

#include "zypp/parser/tagfile/TagFileParser.h"
#include "zypp/source/susetags/MediaMetadataParser.h"
#include <boost/regex.hpp>

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
  if (!file)
  {
    ZYPP_THROW(Exception("Can't read media file "+file_r.asString()));
  }

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

  if (boost::regex_match(buffer, what_digit, is_digit_rx))
  {
    // it was the media count
    str::strtonum(buffer, entry_r.count);
    // consume another line
    consume = true;
  }
  else
  {
    // media count defaults to 1
    entry_r.count = 1;
  }

  while (file && !file.eof())
  {
    // probably is the first line after we dont find the media number
    if (consume)
      getline(file, buffer);

    // only skip once
    consume = true;
    boost::regex e("^MEDIA([\\d]+)(\\.([_A-Za-z]+)){0,1} (.+)$");
    boost::smatch what;
    if (boost::regex_match(buffer, what, e, boost::match_extra))
    {
      if ( what.size() < 5 )
      {
        ZYPP_THROW (Exception("Can't match MEDIA in '" + buffer + "'"));
      }

      dumpRegexpResults(what);

      unsigned int number = 1;
      str::strtonum( what[1], number);
      std::string lang = what[3];
      std::string desc = what[4];
      entry_r.alternate_names[number][lang] = desc;
    }
    else
    {
      ZYPP_THROW (Exception("Can't find MEDIA in '" + buffer + "'"));
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
