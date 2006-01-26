/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMisc.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H
#define ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H

#include <string>
#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/ResolverInfoContainer.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfoMisc

class ResolverInfoMisc : public ResolverInfoContainer {

	private:

	  Capability _capability;			// capability leading to this info

	  PoolItem_Ref _other_item;
	  Capability _other_capability;

	  std::string _action;
	  std::string _trigger;

	public:

	  ResolverInfoMisc (ResolverInfoType detailedtype, PoolItem_Ref affected, int priority, const Capability & capability = Capability());
	  virtual ~ResolverInfoMisc();

	  // ---------------------------------- I/O

	  friend std::ostream& operator<<(std::ostream&, const ResolverInfoMisc & context);

	  // ---------------------------------- accessors

	  std::string message (void) const;
	  std::string action (void) const { return _action; }
	  std::string trigger (void) const { return _trigger; }

	  PoolItem_Ref other (void) const { return _other_item; }
	  const Capability other_capability (void) const { return _other_capability; }
	  const Capability capability(void) const { return _capability; }

	  // ---------------------------------- methods

	  virtual bool merge (ResolverInfo_Ptr to_be_merged);
	  virtual ResolverInfo_Ptr copy (void) const;

	  void addAction (const std::string & action_msg);
	  void addTrigger (const std::string & trigger_msg);

	  void setOtherPoolItem_Ref (PoolItem_Ref other);
	  void setOtherCapability (const Capability & capability);
};

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////        
#endif // ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H
 
