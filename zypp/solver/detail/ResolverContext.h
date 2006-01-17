/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverContext.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERCONTEXT_H
#define ZYPP_SOLVER_DETAIL_RESOLVERCONTEXT_H

#include <iosfwd>
#include <list>
#include <map>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/ResolverContextPtr.h"
#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/Channel.h"
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

typedef enum {
    RESOLVABLE_STATUS_UNKNOWN = 0,
    RESOLVABLE_STATUS_INSTALLED,
    RESOLVABLE_STATUS_SATISFIED,
    RESOLVABLE_STATUS_UNNEEDED,
    RESOLVABLE_STATUS_INCOMPLETE,
    RESOLVABLE_STATUS_UNINSTALLED,
    RESOLVABLE_STATUS_TO_BE_INSTALLED,
    RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK
} ResItemStatus;

#define resItem_status_is_to_be_installed(x) (((x) == RESOLVABLE_STATUS_TO_BE_INSTALLED) || ((x) == RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT))
#define resItem_status_is_to_be_uninstalled(x) (((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED) || ((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE) || ((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK))
#define resItem_status_is_satisfied(x) ((x) == RESOLVABLE_STATUS_SATISFIED)
#define resItem_status_is_incomplete(x) ((x) == RESOLVABLE_STATUS_INCOMPLETE)
#define resItem_status_is_unneeded(x) ((x) == RESOLVABLE_STATUS_UNNEEDED)

typedef std::map<ResItem_constPtr, ResItemStatus> StatusTable;
typedef std::list<ResolverInfo_Ptr> InfoList;


typedef void (*ResolverContextFn) (ResolverContext_Ptr ctx, void * data);
typedef void (*MarkedResItemFn) (ResItem_constPtr res, ResItemStatus status, void *data);
typedef void (*MarkedResItemPairFn) (ResItem_constPtr res1, ResItemStatus status1, ResItem_constPtr res2, ResItemStatus status2, void *data);


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverContext
class ResolverContext : public base::ReferenceCounted, private base::NonCopyable {
    

  private:

    ResolverContext_Ptr _parent;

    int _refs;

    World_Ptr _world;
    StatusTable _status;

    // just a caching mechanism
    ResItem_constPtr _last_checked_resItem;
    ResItemStatus _last_checked_status;

    InfoList _log;
    unsigned long long _download_size;
    unsigned long long _install_size;
    int _total_priority;
    int _min_priority;
    int _max_priority;
    int _other_penalties;
    Channel_constPtr _current_channel;
    bool _verifying;
    bool _invalid;

  public:
    ResolverContext (ResolverContext_Ptr parent = NULL);
    virtual ~ResolverContext();

    // ---------------------------------- I/O

    static std::string toString (const ResolverContext & context);
    virtual std::ostream & dumpOn(std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream&, const ResolverContext & context);
    std::string asString (void ) const;

    static std::string toString (const ResItemStatus & status);

    // ---------------------------------- accessors

    World_Ptr world (void) const;				// gets global world, if _world == NULL
    void setWorld (World_Ptr world) { _world = world; }

    Channel_constPtr currentChannel (void) const { return _current_channel; }
    void setCurrentChannel (Channel_constPtr channel) { _current_channel = channel; }

    unsigned long long downloadSize(void) const { return _download_size; }
    unsigned long long installSize(void) const { return _install_size; }
    int totalPriority (void) const { return _total_priority; }
    int minPriority (void) const { return _min_priority; }
    int maxPriority (void) const { return _max_priority; }
    int otherPenalties (void) const { return _other_penalties; }

    bool isValid (void) const { return !_invalid; }
    bool isInvalid (void) const { return _invalid; }

    bool verifying (void) const { return _verifying; }
    void setVerifying (bool verifying) { _verifying = verifying; }

    // ---------------------------------- methods

    ResItemStatus getStatus (ResItem_constPtr res);			// non-const, because its caching
    void setStatus (ResItem_constPtr res, ResItemStatus status);

    // state change functions
    //   they do some checking before calling setStatus()
    bool installResItem (ResItem_constPtr resItem, bool is_soft, int other_penalty);
    bool satisfyResItem (ResItem_constPtr resItem, int other_penalty);
    bool unneededResItem (ResItem_constPtr resItem, int other_penalty);
    bool incompleteResItem (ResItem_constPtr resItem, int other_penalty);
    bool upgradeResItem (ResItem_constPtr new_resItem, ResItem_constPtr old_resItem, bool is_soft, int other_penalty);
    bool uninstallResItem (ResItem_constPtr resItem, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink);

    bool resItemIsPresent (ResItem_constPtr resItem);
    bool resItemIsAbsent (ResItem_constPtr resItem);

    void foreachMarkedResItem (MarkedResItemFn fn, void *data) const;
    CResItemList getMarkedResItems (int which) const;				// <0:uninstalls, 0:all; >0:installs

    int foreachInstall (MarkedResItemFn fn, void *data) const;
    CResItemList getInstalls (void) const;
    int installCount (void) const;

    int foreachUninstall (MarkedResItemFn fn, void *data);			// non-const, calls foreachUpgrade
    CResItemList getUninstalls (void);
    int uninstallCount (void);

    int foreachUpgrade (MarkedResItemPairFn fn, void *data);			// non-const, calls getStatus
    CResItemList getUpgrades (void);
    int upgradeCount (void);

    int foreachSatisfy (MarkedResItemFn fn, void *data);			// non-const, calls getStatus
    CResItemList getSatisfies (void);
    int satisfyCount (void);

    int foreachIncomplete (MarkedResItemFn fn, void *data);			// non-const, calls getStatus
    CResItemList getIncompletes (void);
    int incompleteCount (void);

    void addInfo (ResolverInfo_Ptr info);
    void addInfoString (ResItem_constPtr resItem, int priority, std::string str);
    void addErrorString (ResItem_constPtr resItem, std::string str);

    void foreachInfo (ResItem_Ptr resItem, int priority, ResolverInfoFn fn, void *data);
    InfoList getInfo (void);

    void spew (void);
    void spewInfo (void);

    bool requirementIsMet (const Capability & dep, bool is_child = false);
    bool requirementIsPossible (const Capability & dep);
    bool resItemIsPossible (ResItem_constPtr resItem);
    bool isParallelInstall (ResItem_constPtr resItem);

    int getChannelPriority (Channel_constPtr channel) const;

    int partialCompare (ResolverContext_Ptr context);			// non-const, calls uninstall/upgrade Count
    int compare (ResolverContext_Ptr context);
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
#endif // ZYPP_SOLVER_DETAIL_RESOLVERCONTEXT_H

