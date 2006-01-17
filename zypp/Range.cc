/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Range.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/Range.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace range_detail
  {
    /** Compute Range overlaps.
     * Takes the \a lhs and \a rhs operator and the result
     * of comparing \a lhs and \a rhs (<tt>-1,0,1</tt>).
     *
    */
    bool overlaps( Rel lhs, Rel rhs, int cmp )
    {
      if ( lhs == Rel::NONE || rhs == Rel::NONE )
        return false;
      if ( lhs == Rel::ANY || rhs == Rel::ANY )
        return true;

      if ( lhs == Rel::NE )
      {
	  if ( cmp < 0 )
	  {
	      // lhs < rhs
	      return( rhs == Rel::GE 
		      || rhs == Rel::EQ );
	  } else if ( cmp > 0)
	  {
	      // lhs > rhs
	      return( rhs == Rel::LT
		      || rhs == Rel::EQ );	      
	  } else 
	  {
	      //lhs == rhs
	      return ( rhs == Rel::GT
		       || rhs == Rel::LT );
	  }
      }
      
      if ( rhs == Rel::NE )
      {
	  if ( cmp < 0 )
	  {
	      // lhs < rhs
	      return(  lhs == Rel::LE
		       || lhs == Rel::EQ );
	  } else if ( cmp > 0)
	  {
	      // lhs > rhs
	      return(  lhs == Rel::GT
		       || lhs == Rel::EQ );	      
	  } else
	  {
	      //lhs == rhs
	      return ( lhs == Rel::GT
		       || lhs == Rel::LT );
	  }
      }

      if ( cmp < 0 )
        {
          // lhs < rhs: either lhs includes greater values or rhs includes lower.
          return(    lhs == Rel::GT
                  || lhs == Rel::GE
                  || rhs == Rel::LT
                  || rhs == Rel::LE );
        }

      if ( cmp > 0 )
        {
          // lhs > rhs: either lhs includes lower values or rhs includes greater.
          return(    lhs == Rel::LT
                  || lhs == Rel::LE
                  || rhs == Rel::GT
                  || rhs == Rel::GE );
        }

      // lhs == rhs: either both ranges include Rel::EQ, or both head
      // into the same direction.
      if (    ( lhs == Rel::LE || lhs == Rel::EQ || lhs == Rel::GE )
           && ( rhs == Rel::LE || rhs == Rel::EQ || rhs == Rel::GE ) )
        return true;
      if (    ( lhs == Rel::LT && ( rhs == Rel::LT || rhs == Rel::LE ) )
           || ( lhs == Rel::GT && ( rhs == Rel::GT || rhs == Rel::GE ) )
           || ( rhs == Rel::LT && ( lhs == Rel::LT || lhs == Rel::LE ) )
           || ( rhs == Rel::GT && ( lhs == Rel::GT || lhs == Rel::GE ) ) )
        return true;
      // else
      return false;

    }
  }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
