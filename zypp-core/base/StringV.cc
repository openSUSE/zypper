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
#include <zypp-core/base/StringV.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace strv
  {
    namespace detail
    {
      /** \ref split working horse for empty \a sep_r case.
       * Split at /[BLANK,TAB]+/ and report no-empty words. Trim::right is
       * applied to the rest of the line in case the callback aborts further
       * processing.
       */
      unsigned _splitSimple( std::string_view line_r, WordConsumer && fnc_r )
      {
	// callback stats
	unsigned fncCall = 0;

	// NOTE: line_r is not null-terminated!
	const char *const eol = line_r.data() + line_r.size();	// the '\0'
	const char * searchfrom = line_r.data();	// start of the next search (moves with each cycle!)

	// Empty sep: split at /[BLANK,TAB]+/ and report no-empty words
	auto isSep = []( char ch )->bool { return ch == ' '|| ch == '\t'; };

	auto skipSep = [eol,&isSep]( const char *& ptr )->bool {
	  while ( ptr < eol && isSep( *ptr ) )
	    ++ptr;
	  return ptr < eol;	// whether we ended up at a new wordstart
	};

	auto skipWord = [eol,&isSep]( const char *& ptr )->void {
	  while ( ptr < eol && ! isSep( *ptr ) )
	    ++ptr;
	};

	// For the 'last' CB argument: we must remember a word
	// until we know whether another one is following
	std::string_view word;

	while ( skipSep( searchfrom ) ) {
	  const char * wordstart = searchfrom;
	  // Report a previous word found
	  if ( ! word.empty() ) {
	    if ( fnc_r ) {
	      if ( ! fnc_r( word, fncCall, false/*more to come*/ ) ) {
		// Stop searching for further matches. Final report will
		// be the remaining line (right trimmed)
		const char * wordend = eol;
		while ( isSep( *(wordend-1) ) ) // noSep at wordstart stops loop
		  --wordend;
		word = std::string_view( wordstart, wordend-wordstart );
		++fncCall;
		break;
	      }
	    }
	    ++fncCall;
	  }
	  // remember new word
	  ++searchfrom;
	  skipWord( searchfrom );
	  word = std::string_view( wordstart, searchfrom-wordstart );
	}

	// finally report the last word
	if ( ! word.empty() ) {
	  if ( fnc_r ) {
	    fnc_r( word, fncCall, true/*last*/ );
	  }
	  ++fncCall;
	}

	return fncCall;
      }
    } // namespace detail

    unsigned detail::_split( std::string_view line_r, std::string_view sep_r, Trim trim_r, WordConsumer && fnc_r )
    {
      if ( sep_r.empty() )
	return _splitSimple( line_r, std::move( fnc_r ) );

      // callback stats
      bool fncStop = false;
      unsigned fncCall = 0;

      using size_type = std::string_view::size_type;
      size_type wordstart = 0;	// start of the next string to be reported
      size_type searchfrom = 0; // start of the next search for a separator

      do {	// report lhs word of separator matches...
	searchfrom = line_r.find( sep_r, searchfrom );
	if ( fncStop || searchfrom == line_r.npos ) {
	  break;
	}

	// Report lhs word of the match and advance...
	if ( fnc_r ) {
	  if ( ! fnc_r( trim( line_r.substr(wordstart,searchfrom-wordstart), trim_r ), fncCall, false/*more to come*/ ) )
	    fncStop= true;	// stop searching for further matches
	}
	++fncCall;

	// Next wordstart is behind the separator match.
	searchfrom += sep_r.size();
	wordstart = searchfrom;
      } while( wordstart < line_r.size() );

      // finally report rhs word of the last separator match (or no match)
      if ( fnc_r ) {
	if ( wordstart < line_r.size() )
	  fnc_r( trim( line_r.substr(wordstart,line_r.size()-wordstart), trim_r ), fncCall, true/*last*/ );
	else	// a final match at $ so a final empty word reported (no trim needed)
	  fnc_r( std::string_view( line_r.data()+line_r.size(), 0 ), fncCall, true/*last*/ );
      }
      ++fncCall;
      return fncCall;
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
