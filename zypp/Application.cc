/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Application.cc
 */
#include <iostream>

//#include "zypp/base/LogTools.h"
#include "zypp/Application.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  IMPL_PTR_TYPE( Application );

  Application::Application( const sat::Solvable & solvable_r )
    : ResObject( solvable_r )
  {}

  Application::~Application()
  {}

} // namespace zypp
///////////////////////////////////////////////////////////////////
