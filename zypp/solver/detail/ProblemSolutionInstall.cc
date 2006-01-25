
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

#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/solver/detail/ProblemSolutionInstall.h"

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

IMPL_PTR_TYPE(ProblemSolutionInstall);

//---------------------------------------------------------------------------

ProblemSolutionInstall::ProblemSolutionInstall( ResolverProblem_Ptr parent,
						ResItem_constPtr resItem )
    : ProblemSolution (parent, "", "")
{
    // TranslatorExplanation %s = name of package, patch, selection ...    
    _description = str::form (_("Installing %s"), resItem->name().c_str() );
    // TranslatorExplanation %s = name of package, patch, selection ...      
    _details = str::form (_("Installing %s"), resItem->asString().c_str() );

    addAction ( new TransactionSolutionAction (resItem,
					       INSTALL));
}

ProblemSolutionInstall::ProblemSolutionInstall( ResolverProblem_Ptr parent,
						CResItemList & resItemList )
    : ProblemSolution (parent, "", "")
{
    _description = _("Installing missing resolvables");

    for (CResItemList::const_iterator iter = resItemList.begin();
	 iter != resItemList.end(); iter++) {
	ResItem_constPtr resItem = *iter;
	addAction ( new TransactionSolutionAction (resItem, INSTALL));
    }
    
    _details = SolutionAction::toString (_actions);
    
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
