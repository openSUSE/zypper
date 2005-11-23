/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       FSize.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/

#include <stdlib.h>

#include <iostream>

#include <y2util/stringutil.h>
#include <y2util/FSize.h>

using namespace std;

FSize::FSize( const string &sizeStr, const Unit unit_r )
  : _size( atoll( sizeStr.c_str() ) * factor( unit_r ) )
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : FSize::fillBlock
//	METHOD TYPE : FSize &
//
//	DESCRIPTION :
//
FSize & FSize::fillBlock( FSize blocksize_r )
{
  if ( _size && blocksize_r ) {
    long long diff = _size % blocksize_r;
    if ( diff ) {
      if ( _size > 0 )
	_size += blocksize_r;
      _size -= diff;
    }
  }
  return *this;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : FSize::bestUnit
//	METHOD TYPE : FSize::Unit
//
//	DESCRIPTION :
//
FSize::Unit FSize::bestUnit() const
{
  long long usize( _size < 0 ? -_size : _size );
  if ( usize < KB )
    return B;
  if ( usize < MB )
    return K;
  if ( usize < GB )
    return M;
  if ( usize < TB )
    return G;
  return T;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : FSize::form
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string FSize::form( const Unit unit_r, unsigned fw, unsigned prec, const bool showunit ) const
{
  if ( prec == bestPrec ) {
    switch ( unit_r )
    {
      case T:  prec = 3; break;
      case G:  prec = 2; break;
      case M:  prec = 1; break;
      case K:  prec = 1; break;
      case B:  prec = 0; break;
    }
  } else if ( unit_r == B )
    prec = 0; // doesn't make sense for Byte

  string ret = stringutil::form( "%*.*f", fw, prec, ( double( _size ) / factor( unit_r ) ) );
  if ( showunit ) {
    ret = stringutil::form( "%s %s", ret.c_str(), unit( unit_r ) );
  }
  return ret;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : FSize::asString
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string FSize::asString() const
{
  return form();
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream & operator<<( std::ostream & str, const FSize & obj )
{
  return str << obj.asString();
}

