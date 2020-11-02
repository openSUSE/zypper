/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/StringV.cc
 */

#include <zypp/base/StringV.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace strv
  {

    unsigned split( std::string_view line_r, std::string_view sep_r, Trim trim_r,
		    std::function<void(std::string_view)> fnc_r )
    {
#warning REIMPLEMENT
      std::vector<std::string> words;
      str::split( std::string(line_r), std::back_inserter(words), std::string(sep_r), str::TRIM );

      if ( fnc_r ) {
	for ( const auto & w : words )
	  fnc_r( std::string_view(w) );
      }
      return words.size();
    }

  } // namespace strv
} // namespace zypp
///////////////////////////////////////////////////////////////////
