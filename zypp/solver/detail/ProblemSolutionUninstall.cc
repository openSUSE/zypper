
/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ProblemSolution.cc
 *
 * Easy-to use interface to the ZYPP dependency resolver
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
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

#include <sstream>

#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/solver/detail/ProblemSolutionUninstall.h"
#include "zypp/solver/detail/ResolverInfo.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

IMPL_PTR_TYPE(ProblemSolutionUninstall);

//---------------------------------------------------------------------------
ProblemSolutionUninstall::ProblemSolutionUninstall( ResolverProblem_Ptr parent, PoolItem_Ref item,
			  const std::string & descr,
			  const std::string & detail)
    : ProblemSolution (parent, descr, detail)
{
    addAction ( new TransactionSolutionAction (item, REMOVE));
}

	

ProblemSolutionUninstall::ProblemSolutionUninstall( ResolverProblem_Ptr parent,
						    PoolItem_Ref item)
    : ProblemSolution (parent, "", "")
{
    ResStatus status = item.status();
    if (status.isInstalled()) {
	// TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form (_("delete %s"), item->name().c_str() );
    	// TranslatorExplanation %s = name of package, patch, selection ...	    
	_details = str::form (_("delete %s"), ResolverInfo::toString (item).c_str());
    } else {
	// TranslatorExplanation %s = name of package, patch, selection ...	
	_description = str::form (_("do not install %s"), item->name().c_str() );
    	// TranslatorExplanation %s = name of package, patch, selection ...	    
	_details = str::form (_("do not install %s"), ResolverInfo::toString (item).c_str());
    }

    addAction ( new TransactionSolutionAction (item, REMOVE));
}

ProblemSolutionUninstall::ProblemSolutionUninstall( ResolverProblem_Ptr parent,
						    PoolItemList & itemlist)
    : ProblemSolution (parent, "", "")
{
    _description = _("Do not install or delete the resolvables concerned");

    for (PoolItemList::iterator iter = itemlist.begin();
	 iter != itemlist.end(); iter++) {
	PoolItem_Ref item = *iter;
	addAction ( new TransactionSolutionAction (item, REMOVE));
    }
    
    ostringstream details;
    details << _actions;    
    _details = details.str();
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
