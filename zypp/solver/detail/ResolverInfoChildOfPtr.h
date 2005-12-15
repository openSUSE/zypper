/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoChildOfPtr.h
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

#ifndef _ResolverInfoChildOfPtr_h
#define _ResolverInfoChildOfPtr_h

#include <y2util/RepDef.h>
#include <zypp/solver/detail/ResolverInfoPtr.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//	CLASS NAME : ResolverInfoChildOfPtr
//	CLASS NAME : constResolverInfoChildOfPtr
///////////////////////////////////////////////////////////////////
DEFINE_DERIVED_POINTER(ResolverInfoChildOf, ResolverInfo);

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // _ResolverInfoChildOfPtr_h
