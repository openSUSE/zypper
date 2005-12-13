/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ByteCount.cc
 *
*/
#include <iostream>

#include "zypp/ByteCount.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  const ByteCount::Unit ByteCount::B( 1, "B", 0 );
  const ByteCount::Unit ByteCount::K( 1024, "K", 1 );
  const ByteCount::Unit ByteCount::M( 1048576, "M", 1 );
  const ByteCount::Unit ByteCount::G( 1073741824, "G", 2 );
  const ByteCount::Unit ByteCount::T( 1099511627776, "T", 3 );

  const ByteCount::Unit ByteCount::kB( 1000, "kB", 1 );
  const ByteCount::Unit ByteCount::MB( 1000000, "MB", 1 );
  const ByteCount::Unit ByteCount::GB( 1000000000, "GB", 2 );
  const ByteCount::Unit ByteCount::TB( 1000000000000, "TB", 3 );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ByteCount::fillBlock
  //	METHOD TYPE : ByteCount &
  //
  ByteCount & ByteCount::fillBlock( ByteCount blocksize_r )
  {
    if ( _count && blocksize_r )
      {
        SizeType diff = _count % blocksize_r;
        if ( diff )
          {
            if ( _count > 0 )
              _count += blocksize_r;
            _count -= diff;
          }
      }
    return *this;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ByteCount::bestUnit
  //	METHOD TYPE : ByteCount::Unit
  //
  const ByteCount::Unit & ByteCount::bestUnit() const
  {
    SizeType usize( _count < 0 ? -_count : _count );
    if ( usize < K.factor() )
      return B;
    if ( usize < M.factor() )
      return K;
    if ( usize < G.factor() )
      return M;
    if ( usize < T.factor() )
      return G;
    return T;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ByteCount::bestUnit1000
  //	METHOD TYPE : ByteCount::Unit
  //
  const ByteCount::Unit & ByteCount::bestUnit1000() const
  {
    SizeType usize( _count < 0 ? -_count : _count );
    if ( usize < kB.factor() )
      return B;
    if ( usize < MB.factor() )
      return kB;
    if ( usize < GB.factor() )
      return MB;
    if ( usize < TB.factor() )
      return GB;
    return TB;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
