/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 \file	zypp/ResName.cc

 \brief	.

*/
#include <iostream>

#include "zypp/ResName.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResName::ResName
  //	METHOD TYPE : Ctor
  //
  ResName::ResName()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResName::ResName
  //	METHOD TYPE : Ctor
  //
  ResName::ResName( const std::string & rhs )
  : base::StringVal( rhs )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResName::ResName
  //	METHOD TYPE : Ctor
  //
  ResName::ResName( const ResName & rhs )
  : base::StringVal( rhs )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResName::~ResName
  //	METHOD TYPE : Dtor
  //
  ResName::~ResName()
  {}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
