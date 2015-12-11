/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <cstring>
#include <boost/utility/string_ref.hpp>
#include "utils/text.h"

std::string mbs_substr_by_width( boost::string_ref text_r, std::string::size_type colpos_r, std::string::size_type collen_r )
{
  std::string ret;
  if ( collen_r )
  {
    const char * spos	= nullptr;
    size_t slen		= 0;

    size_t colend = ( collen_r == std::string::npos ? std::string::npos : colpos_r+collen_r ); // will exploit npos == size_t(-1)
    size_t pos = 0;
    for( mbs::MbsIterator it( text_r ); ! it.atEnd(); ++it )
    {
      // collect sequences [pos,end[ in [colpos_r,colend[
      // partial overlaps are padded
      size_t end = pos + it.columns();

      if ( pos < colpos_r )	// starts before range
      {
	if ( end > colpos_r )	// pad incomplete sequence at range begin
	  ret += std::string( std::min(end,colend)-colpos_r, ' ' );
      }
      else 			// starts inside range (pos < colend by the way we loop)
      {
	if ( end <= colend )	// completely inside
	{
	  if ( !spos )
	    spos = it.pos();
	  slen += it.size();
	}
	else			// partial outside
	{
	  if ( spos )
	  {
	    ret += std::string( spos, slen );
	    spos = nullptr;
	    slen = 0;		// don't collect it after loop
	  }
	  ret += std::string( colend-pos, ' ' );
	  break;		// done
	}
      }

      if ( end >= colend )
	break;
      pos = end;
    }
    if ( spos )
      ret += std::string( spos, slen );
  }
  return ret;
}
