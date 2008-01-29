
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
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/solver/detail/ProblemSolutionUnlock.h"

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

IMPL_PTR_TYPE(ProblemSolutionUnlock);

//---------------------------------------------------------------------------

struct LockReset : public resfilter::PoolItemFilterFunctor
{
    ProblemSolutionUnlock & _problemSolutionUnlock;
    LockReset( ProblemSolutionUnlock & solution )
	: _problemSolutionUnlock( solution )
    { }

    bool operator()( PoolItem item )
    {
	_problemSolutionUnlock.addAction ( new TransactionSolutionAction (item, UNLOCK));	
	return true;
    }
};

	
ProblemSolutionUnlock::ProblemSolutionUnlock( ResolverProblem_Ptr parent,
					      const ResPool & pool)
    : ProblemSolution (parent, "", "")
{
    _description = _("unlock all resolvables");
    LockReset lockReset (*this);

    invokeOnEach ( pool.begin(), pool.end(),
		   resfilter::ByLock( ),
		   functor::functorRef<bool,PoolItem>(lockReset));
}
	
ProblemSolutionUnlock::ProblemSolutionUnlock( ResolverProblem_Ptr parent,
					      PoolItem item)
    : ProblemSolution (parent, "", "")
{
    // TranslatorExplanation %s = name of package, patch, selection ...	
    _description = str::form (_("unlock %s"), item->name().c_str() );

    addAction ( new TransactionSolutionAction (item, UNLOCK));
}

ProblemSolutionUnlock::ProblemSolutionUnlock( ResolverProblem_Ptr parent,
					      PoolItemList & itemlist)
    : ProblemSolution (parent, "", "")
{
    _description = _("Unlock these resolvables");

    for (PoolItemList::iterator iter = itemlist.begin();
	 iter != itemlist.end(); iter++) {
	PoolItem item = *iter;
	addAction ( new TransactionSolutionAction (item, UNLOCK));
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
