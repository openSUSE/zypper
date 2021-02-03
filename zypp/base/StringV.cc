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
#include <iostream>
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


    unsigned detail::_splitRx( const std::string & line_r, const regex & rx_r, WordConsumer && fnc_r )
    {
      // callback stats
      bool fncStop = false;
      unsigned fncCall = 0;

      // report lhs word of separator matches...
      const char *const eol = line_r.data() + line_r.size();	// the '\0'
      bool trailingNL = line_r.size() && *(eol-1) == '\n';	// line ends with NL
      const char * wordstart = line_r.data();	// start of the next string to be reported
      const char * searchfrom = line_r.data();	// start of the next search for a separator (moves with each cycle!)

      // Whether to match the ^ at beginning of the line or after an \n:
      auto matchAtBOL = [&]() {
	return searchfrom == line_r.data() || *(searchfrom-1) == '\n' ? regex::none : regex::not_bol;
      };
      do {	// report lhs word of separator matches...
	smatch match;
	if ( fncStop || ! rx_r.matches( searchfrom, match, matchAtBOL() ) ) {
	  break;
	}
	if ( trailingNL && searchfrom+match.begin(0) == eol )
	  break;	// don't report matches behind a trailing NL

	if ( match.end(0) == 0 && searchfrom == wordstart && searchfrom != line_r.data() ) {
	  // An empty(!) separator found at wordstart is ignored unless we're at the very beginning.
	  // If searchfrom == wordstart we just skipped over a not-empty separator. If wordstart is
	  // not part of a 2nd not-empty separator, we want the next separator to it's right.
	  // Example: Rx:"b*" Str:"0b2" - at pos 2 Rx matches empty; the previous cycle found 'b' and reported the '0'.
	  ++searchfrom;
	} else {
	  // Report lhs word of the match and advance...
	  if ( fnc_r ) {
	    if ( ! fnc_r( std::string_view( wordstart, searchfrom+match.begin(0) - wordstart ), fncCall, false/*more to come*/ ) )
	      fncStop= true;	// stop searching for further matches
	  }
	  ++fncCall;
	  // Next wordstart is behind the separator match.
	  // Next searchfrom also, but advances at least by 1.
	  wordstart = searchfrom+match.end(0);
	  searchfrom += match.end(0) ? match.end(0) : 1;
	}
      } while ( searchfrom <= eol );	// incl. '== eol' as there might be an (empty) match at $

      // finally report rhs word of the last separator match (or no match)
      if ( fnc_r ) {
	if ( wordstart < eol )
	  fnc_r( std::string_view( wordstart, eol-wordstart ), fncCall, true/*last*/ );
	else	// a final match at $ so a final empty word reported
	  fnc_r( std::string_view( eol, 0 ), fncCall, true/*last*/ );
      }
      ++fncCall;
      return fncCall;
    }

  } // namespace strv
} // namespace zypp
///////////////////////////////////////////////////////////////////
