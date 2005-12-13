/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemRequire.h
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

#ifndef _QueueItemRequire_h
#define _QueueItemRequire_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/QueueItemRequirePtr.h>
#include <zypp/solver/detail/Resolvable.h>
#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/Channel.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {


//////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : QueueItemRequire

class QueueItemRequire : public QueueItem {
    REP_BODY(QueueItemRequire);

  private:
    constDependencyPtr _dep;
    constResolvablePtr _requiring_resolvable;
    constResolvablePtr _upgraded_resolvable;
    constResolvablePtr _lost_resolvable;
    bool _remove_only;
    bool _is_child;

  public:

    QueueItemRequire (WorldPtr world, constDependencyPtr dep);
    virtual ~QueueItemRequire();

    // ---------------------------------- I/O

    static std::string toString (const QueueItemRequire & item);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const QueueItemRequire & item);

    std::string asString (void ) const;

    // ---------------------------------- accessors

    constDependencyPtr dependency (void) const { return _dep; }

    void setRemoveOnly (void) { _remove_only = true; }
    void setUpgradedResolvable (constResolvablePtr upgraded_resolvable) { _upgraded_resolvable = upgraded_resolvable; }
    void setLostResolvable (constResolvablePtr lost_resolvable) { _lost_resolvable = lost_resolvable; }

    // ---------------------------------- methods

    virtual bool process (ResolverContextPtr context, QueueItemList & qil);
    virtual QueueItemPtr copy (void) const;
    virtual int cmp (constQueueItemPtr item) const;
    virtual bool isRedundant (ResolverContextPtr context) const { return false; }
    virtual bool isSatisfied (ResolverContextPtr context) const { return false; }

    void addResolvable (constResolvablePtr resolvable);


};

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _QueueItemRequire_h
