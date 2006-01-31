/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Types.h
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

#ifndef ZYPP_PROBLEMTYPES_H
#define ZYPP_PROBLEMTYPES_H

#include <iosfwd>
#include <list>
#include <set>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Functional.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
    
    DEFINE_PTR_TYPE(Resolver);
    
    DEFINE_PTR_TYPE(ProblemSolution);
    typedef std::list<ProblemSolution_Ptr> ProblemSolutionList;
    typedef std::list<ProblemSolution_constPtr> CProblemSolutionList;
    
    DEFINE_PTR_TYPE(ResolverProblem);
    typedef std::list<ResolverProblem_Ptr> ResolverProblemList;
    typedef std::list<ResolverProblem_constPtr> CResolverProblemList;

  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_DETAIL_TYPES_H
