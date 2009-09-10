/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/* ResolverUpgrade.cc
 *
 * Implements the distribution upgrade algorithm.
 *
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "zypp/base/LogTools.h"
#include "zypp/base/Exception.h"
#include "zypp/ResPool.h"
#include "zypp/ZYppFactory.h"
#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Testcase.h"
#include "zypp/solver/detail/SATResolver.h"
#include "zypp/Target.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;
using namespace zypp;

//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Resolver::doUpgrade
//	METHOD TYPE : bool
//
//	DESCRIPTION : creates a testcase in /var/log/updateTestcase
//		      and set the SAT solver in update mode
//
bool
Resolver::doUpgrade()
{
  Target_Ptr target( getZYpp()->getTarget() );
  if ( ! target )
  {
    WAR << "Huh, no target ?" << endl;
  }

  // Setting Resolver to upgrade mode. SAT solver will do the update
  _upgradeMode = true;
  return resolvePool();
}


///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////


