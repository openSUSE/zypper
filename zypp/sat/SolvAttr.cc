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

  const SolvAttr SolvAttr::summary      ( "solvable:summary" );
  const SolvAttr SolvAttr::description  ( "solvable:description" );
  const SolvAttr SolvAttr::insnotify    ( "solvable:messageins" );
  const SolvAttr SolvAttr::delnotify    ( "solvable:messagedel" );
  const SolvAttr SolvAttr::vendor       ( "solvable:vendor" );
  const SolvAttr SolvAttr::license      ( "solvable:license" );
  const SolvAttr SolvAttr::size         ( "solvable:installsize" );
  const SolvAttr SolvAttr::downloadsize ( "solvable:downloadsize" );
  
  //package
  const SolvAttr SolvAttr::medianr	( "solvable:medianr" );
  const SolvAttr SolvAttr::mediafile	( "solvable:mediafile" );
  const SolvAttr SolvAttr::mediadir	( "solvable:mediadir" );
  const SolvAttr SolvAttr::eula		( "solvable:eula" );
  const SolvAttr SolvAttr::changelog    ( "changelog" );
  const SolvAttr SolvAttr::buildhost    ( "buildhost" );
  const SolvAttr SolvAttr::distribution ( "distribution" );
  const SolvAttr SolvAttr::packager     ( "packager" );
  const SolvAttr SolvAttr::group        ( "solvable:group" );
  const SolvAttr SolvAttr::keywords     ( "solvable:keywords" );
  const SolvAttr SolvAttr::os           ( "os" );
  const SolvAttr SolvAttr::prein        ( "prein" );
  const SolvAttr SolvAttr::postin       ( "postin" );
  const SolvAttr SolvAttr::preun        ( "preun" );
  const SolvAttr SolvAttr::postun       ( "postun" );
  const SolvAttr SolvAttr::sourcesize   ( "sourcesize" );
  const SolvAttr SolvAttr::authors      ( "solvable:authors" );
  const SolvAttr SolvAttr::filenames    ( "filenames" );
  const SolvAttr SolvAttr::srcpkgname   ( "srcpkgname" );
  const SolvAttr SolvAttr::srcpkgedition( "srcpkgedition" );

  //pattern
  const SolvAttr SolvAttr::isvisible    ( "solvable:isvisible" );
  const SolvAttr SolvAttr::icon         ( "icon" );
  const SolvAttr SolvAttr::isdefault    ( "isdefault" );
  const SolvAttr SolvAttr::category     ( "solvable:category" ); // FIXME translate
  const SolvAttr SolvAttr::script       ( "script" );
 
  
} // namespace sat
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
