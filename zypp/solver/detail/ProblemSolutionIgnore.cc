
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
#include "zypp/base/Logger.h"
#include "zypp/solver/detail/ProblemSolutionIgnore.h"
#include "zypp/solver/detail/Helper.h"

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

IMPL_PTR_TYPE(ProblemSolutionIgnoreConflicts);
IMPL_PTR_TYPE(ProblemSolutionIgnoreRequires);
IMPL_PTR_TYPE(ProblemSolutionIgnoreArchitecture);
IMPL_PTR_TYPE(ProblemSolutionIgnoreVendor);	
IMPL_PTR_TYPE(ProblemSolutionIgnoreInstalled);	

//---------------------------------------------------------------------------

ProblemSolutionIgnoreRequires::ProblemSolutionIgnoreRequires( ResolverProblem_Ptr parent,
							      PoolItem_Ref item,
							      const Capability & capability)
    : ProblemSolution (parent, "", "")
{
	_description = _("Ignore this requirement just here");
	addAction ( new InjectSolutionAction (item, capability, REQUIRES));
}	
	
ProblemSolutionIgnoreRequires::ProblemSolutionIgnoreRequires( ResolverProblem_Ptr parent,
							      PoolItemList itemList,	  
							      const Capability & capability)
    : ProblemSolution (parent, "", "")
{
	_description = _("Generally ignore this requirement");
	for (PoolItemList::const_iterator iter = itemList.begin();
	     iter != itemList.end(); iter++) {
	    addAction ( new InjectSolutionAction (*iter, capability, REQUIRES));
	}
}

ProblemSolutionIgnoreArchitecture::ProblemSolutionIgnoreArchitecture( ResolverProblem_Ptr parent,
								  PoolItem_Ref item)
    : ProblemSolution (parent, "", "")
{
        // TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form(_("Install %s although it would change the architecture"),
				 item->name().c_str());
	// TranslatorExplanation %s = name of package, patch, selection ...	
	_details = str::form(_("%s provides this dependency, but would change the architecture of the installed item"),
			    Helper::itemToString (item).c_str());
	addAction ( new InjectSolutionAction (item, ARCHITECTURE));
}

ProblemSolutionIgnoreVendor::ProblemSolutionIgnoreVendor( ResolverProblem_Ptr parent,
							  PoolItem_Ref item)
    : ProblemSolution (parent, "", "")
{
        // TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form(_("Install %s although it would change the vendor"),
				 item->name().c_str());
	// TranslatorExplanation %s = name of package, patch, selection ...	
	_details = str::form(_("%s provides this dependency, but would change the vendor of the installed item"),
			    Helper::itemToString (item).c_str());
	addAction ( new InjectSolutionAction (item, VENDOR));
}
	
ProblemSolutionIgnoreConflicts::ProblemSolutionIgnoreConflicts( ResolverProblem_Ptr parent,
								PoolItem_Ref item,
								const Capability & capability,
								PoolItem_Ref otherItem)
    : ProblemSolution (parent, "", "")
{
	// TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form (_("Ignore this conflict of %s"),
				  item->name().c_str());
	addAction (new InjectSolutionAction (item, capability, CONFLICTS, otherItem));	
}

ProblemSolutionIgnoreConflicts::ProblemSolutionIgnoreConflicts( ResolverProblem_Ptr parent,
								PoolItem_Ref item,
								const Capability & capability,
								PoolItemList itemList)
    : ProblemSolution (parent, "", "")
{
	// TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form (_("Ignore this conflict of %s"),
				  item->name().c_str());
	for (PoolItemList::const_iterator iter = itemList.begin();
	     iter != itemList.end(); iter++) {
	    addAction (new InjectSolutionAction (item, capability, CONFLICTS, *iter));		    
	}
}

ProblemSolutionIgnoreObsoletes::ProblemSolutionIgnoreObsoletes( ResolverProblem_Ptr parent,
								PoolItem_Ref item,
								const Capability & capability,
								PoolItem_Ref otherItem)
    : ProblemSolution (parent, "", "")
{
	// TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form (_("Ignore the obsolete %s in %s"),
				  Helper::capToString (capability).c_str(),
				  otherItem->name().c_str());
	addAction (new InjectSolutionAction (item, capability, OBSOLETES, otherItem));	
}


ProblemSolutionIgnoreInstalled::ProblemSolutionIgnoreInstalled( ResolverProblem_Ptr parent,
								PoolItem_Ref item,
								PoolItem_Ref otherItem)
    : ProblemSolution (parent, "", "")
{
	// TranslatorExplanation %s = name of package, patch, selection ...
	_description = str::form (_("Ignore that %s is already set to install"),
				  item->name().c_str());
	addAction (new InjectSolutionAction (item, Capability(), INSTALLED, otherItem));	
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
