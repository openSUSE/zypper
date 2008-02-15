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
  const SolvAttr SolvAttr::size         ( "installsize" );
  const SolvAttr SolvAttr::downloadsize ( "downloadsize" );
  
  //package
  const SolvAttr SolvAttr::medianr	( "medianr" );
  const SolvAttr SolvAttr::mediafile	( "mediafile" );
  const SolvAttr SolvAttr::mediadir	( "mediadir" );
  const SolvAttr SolvAttr::eula		( "eula" );
  const SolvAttr SolvAttr::changelog    ( "changelog" );
  const SolvAttr SolvAttr::buildhost    ( "buildhost" );
  const SolvAttr SolvAttr::distribution ( "distribution" );
  const SolvAttr SolvAttr::packager     ( "packager" );
  const SolvAttr SolvAttr::group        ( "group" );
  const SolvAttr SolvAttr::keywords     ( "keywords" );
  const SolvAttr SolvAttr::os           ( "os" );
  const SolvAttr SolvAttr::prein        ( "prein" );
  const SolvAttr SolvAttr::postin       ( "postin" );
  const SolvAttr SolvAttr::preun        ( "preun" );
  const SolvAttr SolvAttr::postun       ( "postun" );
  const SolvAttr SolvAttr::sourcesize   ( "sourcesize" );
  const SolvAttr SolvAttr::authors      ( "authors" );
  const SolvAttr SolvAttr::filenames    ( "filenames" );
  const SolvAttr SolvAttr::srcpkgname   ( "srcpkgname" );
  const SolvAttr SolvAttr::srcpkgedition( "srcpkgedition" );

  //pattern
  const SolvAttr SolvAttr::isvisible    ( "isvisible" );
  const SolvAttr SolvAttr::icon         ( "icon" );
  const SolvAttr SolvAttr::isdefault    ( "isdefault" );
  const SolvAttr SolvAttr::category     ( "category" ); // FIXME translate
  const SolvAttr SolvAttr::script       ( "script" );
 
  
} // namespace sat
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
