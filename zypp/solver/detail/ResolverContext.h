/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _ResolverContext_h
#define _ResolverContext_h

#include <iosfwd>
#include <list>
#include <map>
#include <string.h>

#include <zypp/solver/detail/ResolverContextPtr.h>
#include <zypp/solver/detail/ResolverInfo.h>
#include <zypp/solver/detail/Resolvable.h>
#include <zypp/solver/detail/Channel.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

typedef enum {
    RESOLVABLE_STATUS_UNKNOWN = 0,
    RESOLVABLE_STATUS_INSTALLED,
    RESOLVABLE_STATUS_UNINSTALLED,
    RESOLVABLE_STATUS_TO_BE_INSTALLED,
    RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE,
    RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK
} ResolvableStatus;

#define resolvable_status_is_to_be_installed(x) (((x) == RESOLVABLE_STATUS_TO_BE_INSTALLED) || ((x) == RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT))
#define resolvable_status_is_to_be_uninstalled(x) (((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED) || ((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE) || ((x) == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK))

typedef std::map<constResolvablePtr, ResolvableStatus> StatusTable;
typedef std::list<ResolverInfoPtr> InfoList;


typedef void (*ResolverContextFn) (ResolverContextPtr ctx, void * data);
typedef void (*MarkedResolvableFn) (constResolvablePtr res, ResolvableStatus status, void *data);
typedef void (*MarkedResolvablePairFn) (constResolvablePtr res1, ResolvableStatus status1, constResolvablePtr res2, ResolvableStatus status2, void *data);


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverContext
class ResolverContext : public CountedRep {
    REP_BODY(ResolverContext);

  private:

    ResolverContextPtr _parent;

    int _refs;
    
    WorldPtr _world;
    StatusTable _status;

    // just a caching mechanism
    constResolvablePtr _last_checked_resolvable;
    ResolvableStatus _last_checked_status;

    InfoList _log;
    unsigned long long _download_size;
    unsigned long long _install_size;
    int _total_priority;
    int _min_priority;
    int _max_priority;
    int _other_penalties;
    constChannelPtr _current_channel;
    bool _verifying;
    bool _invalid;

  public:
    ResolverContext (ResolverContextPtr parent = NULL);
    virtual ~ResolverContext();

    // ---------------------------------- I/O

    static std::string toString (const ResolverContext & context);
    virtual std::ostream & dumpOn(std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream&, const ResolverContext & context);
    std::string asString (void ) const;

    static std::string toString (const ResolvableStatus & status);

    // ---------------------------------- accessors

    WorldPtr world (void) const;				// gets global world, if _world == NULL
    void setWorld (WorldPtr world) { _world = world; }

    constChannelPtr currentChannel (void) const { return _current_channel; }
    void setCurrentChannel (constChannelPtr channel) { _current_channel = channel; }

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

    ResolvableStatus getStatus (constResolvablePtr res);			// non-const, because its caching
    void setStatus (constResolvablePtr res, ResolvableStatus status);

    bool installResolvable (constResolvablePtr resolvable, bool is_soft, int other_penalty);
    bool upgradeResolvable (constResolvablePtr new_resolvable, constResolvablePtr old_resolvable, bool is_soft, int other_penalty);
    bool uninstallResolvable (constResolvablePtr resolvable, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink);

    bool resolvableIsPresent (constResolvablePtr resolvable);
    bool resolvableIsAbsent (constResolvablePtr resolvable);

    void foreachMarkedResolvable (MarkedResolvableFn fn, void *data) const;
    CResolvableList getMarkedResolvables (void) const;

    int foreachInstall (MarkedResolvableFn fn, void *data) const;
    CResolvableList getInstalls (void) const;
    int installCount (void) const;

    int foreachUninstall (MarkedResolvableFn fn, void *data);			// non-const, calls foreachUpgrade
    CResolvableList getUninstalls (void);
    int uninstallCount (void);

    int foreachUpgrade (MarkedResolvablePairFn fn, void *data);			// non-const, calls getStatus
    CResolvableList getUpgrades (void);
    int upgradeCount (void);

    void addInfo (ResolverInfoPtr info);
    void addInfoString (constResolvablePtr resolvable, int priority, std::string str);
    void addErrorString (constResolvablePtr resolvable, std::string str);

    void foreachInfo (ResolvablePtr resolvable, int priority, ResolverInfoFn fn, void *data);
    InfoList getInfo (void);

    void spew (void);
    void spewInfo (void);

    bool requirementIsMet (constDependencyPtr dep, bool is_child);
    bool requirementIsPossible (constDependencyPtr dep);
    bool resolvableIsPossible (constResolvablePtr resolvable);
    bool isParallelInstall (constResolvablePtr resolvable);

    int getChannelPriority (constChannelPtr channel) const;

    int partialCompare (ResolverContextPtr context);			// non-const, calls uninstall/upgrade Count
    int compare (ResolverContextPtr context);
};
     
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////
#endif // _ResolverContext_h
    
