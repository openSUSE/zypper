/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemUninstall.h
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

#ifndef ZYPP_SOLVER_DETAIL_QUEUITEMUNINSTALL_H
#define ZYPP_SOLVER_DETAIL_QUEUITEMUNINSTALL_H

#include "zypp/solver/detail/Types.h"

#include "zypp/solver/detail/QueueItem.h"
#include "zypp/Capability.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

DEFINE_PTR_TYPE(QueueItemUninstall);

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : QueueItemUninstall

class QueueItemUninstall : public QueueItem {

  public:
    typedef enum {
      CONFLICT,				// conflicts [dep]
      OBSOLETE,				// obsolets [dep]
      UNSATISFIED,			// unsatisfied dep, must be unistalled since required dep isnt provided anymore
      BACKOUT,				// back out during verify
      UPGRADE,				// its being upgraded, so the original get uninstalled, find out if this breaks something
      DUPLICATE,			// duplicate install
      EXPLICIT				// user request
    } UninstallReason;


  private:
    PoolItem_Ref _item;			// the item to-be-uninstalled
    UninstallReason _reason;
    bool _soft;
    Capability _cap_leading_to_uninstall;
    PoolItem_Ref _upgraded_to;		// if the uninstall is actually an upgrade

    bool _explicitly_requested;
    bool _remove_only;
    bool _due_to_conflict;
    bool _due_to_obsolete;
    bool _unlink;
    PoolItem_Ref _obsoletes_item; 	// item which has caused (has the obsoletes) this request

  public:

    QueueItemUninstall (const ResPool & pool, PoolItem_Ref item, UninstallReason reason, bool soft = false);
    virtual ~QueueItemUninstall();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream& str, const QueueItemUninstall & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    UninstallReason reason (void) const		{ return _reason; }
    void setCapability (const Capability & cap)	{ _cap_leading_to_uninstall = cap; }
    void setExplicitlyRequested (void)		{ _explicitly_requested = true; }
    void setRemoveOnly (void)			{ _remove_only = true; }
    void setUpgradedTo (PoolItem_Ref item)		{ _upgraded_to = item; }
    void setDueToConflict (void)		{ _due_to_conflict = true; }
    void setDueToObsolete (const PoolItem_Ref item)
	{ _due_to_obsolete = true; _obsoletes_item = item; }
    void setUnlink (void);

    // ---------------------------------- methods

    virtual bool process (ResolverContext_Ptr context, QueueItemList & qil);
    virtual QueueItem_Ptr copy (void) const;
    virtual int cmp (QueueItem_constPtr item) const;
    virtual bool isRedundant (ResolverContext_Ptr context) const { return false; }
    virtual bool isSatisfied (ResolverContext_Ptr context) const { return false; }

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

#endif // ZYPP_SOLVER_DETAIL_QUEUITEMUNINSTALL_H
