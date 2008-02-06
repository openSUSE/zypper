/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolvAttr.cc
 *
*/
#include <iostream>

#include "zypp/base/String.h"

#include "zypp/sat/SolvAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace sat
{ /////////////////////////////////////////////////////////////////

  const SolvAttr SolvAttr::summary      ( "summary" );
  const SolvAttr SolvAttr::description  ( "description" );
  const SolvAttr SolvAttr::insnotify    ( "insnotify" );
  const SolvAttr SolvAttr::delnotify    ( "delnotify" );
  const SolvAttr SolvAttr::vendor       ( "vendor" );
  const SolvAttr SolvAttr::license      ( "license" );
  const SolvAttr SolvAttr::size         ( "size" );
  const SolvAttr SolvAttr::downloadsize ( "downloadsize" );
  
  //package
  const SolvAttr SolvAttr::medianr ( "medianr" );
  const SolvAttr SolvAttr::mediafile ( "mediafile" );
  const SolvAttr SolvAttr::mediadir ( "mediadir" );
  
} // namespace sat
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
