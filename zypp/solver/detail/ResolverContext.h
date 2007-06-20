/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverContext.h
 *
 *Keep a set of 'active' PoolItems, these are part of the current
 *transaction. It thereby provides a to-be status for these items
 *
 *
 *Copyright (C) 2000-2002 Ximian, Inc.
 *Copyright (C) 2005 SUSE Linux Products GmbH
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License,
 *version 2, as published by the Free Software Foundation.
 *
 *This program is distributed in the hope that it will be useful, but
 *WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *02111-1307, USA.
 */

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERCONTEXT_H
#define ZYPP_SOLVER_DETAIL_RESOLVERCONTEXT_H

#include "zypp/ResPool.h"
#include "zypp/PoolItem.h"
#include "zypp/Capability.h"
#include "zypp/Source.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/ResolverInfo.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef void (*ResolverContextFn) (ResolverContext_Ptr ctx, void *data);
typedef void (*MarkedPoolItemFn) (PoolItem_Ref item, const ResStatus & status, void *data);
typedef void (*MarkedPoolItemPairFn) (PoolItem_Ref item1, const ResStatus & status1, PoolItem_Ref item2, const ResStatus & status2, void *data);
typedef std::multimap<PoolItem_Ref,Capability> IgnoreMap;
typedef std::map<Source_Ref ,int> SourceCounter;	

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverContext
class ResolverContext : public base::ReferenceCounted, private base::NonCopyable {
    

  private:

    ResolverContext_Ptr _parent;		// branches share a common parent
    ResolverContext_Ptr _establish_context;   // Context of the last Resolver-Establish call

    typedef std::map<PoolItem_Ref,ResStatus> Context;
    Context _context;				// the set of items touched in this transaction

    ResPool _pool;

    ResolverInfoList _log;			// report log

    unsigned long long _download_size;
    unsigned long long _install_size;
    int _total_priority;
    int _min_priority;
    int _max_priority;
    int _other_penalties;

    bool _verifying;				// running 'verifySystem'
    bool _establishing;				// running 'establishSystem'
    bool _invalid;				// lead to invalid solution
    bool _askUser;				// lead to invalid solution too cause we have to ask the user 

    PoolItem_Ref _last_checked_item;		// cache for {get,set}Status
    ResStatus _last_checked_status;

    PoolItemList _last_getMarked; 	// status of the last getMarked call
                                        // it is sorted
    int _last_getMarked_which;		// which kind of getMarked

    Arch _architecture;

    // These conflict should be ignored of the concering item
    IgnoreMap _ignoreConflicts;
    // These requires should be ignored of the concering item    
    IgnoreMap _ignoreRequires;
    // These obsoletes should be ignored of the concering item    
    IgnoreMap _ignoreObsoletes;    
    // Ignore the status "installed" of the item
    PoolItemList _ignoreInstalledItem;
    // Ignore the architecture of the item
    PoolItemList _ignoreArchitectureItem;

    // Items which has been selected NOT by the solver
    // This will be stored while a solver run in order to save time
    // if this information is needed.
    PoolItemList _userDeleteItems;
    PoolItemList _userInstallItems;
    PoolItemList _userLockUninstalledItems;

    bool _forceResolve; // remove items which are conflicts with others or
                        // have unfulfilled requirements.
                        // This behaviour is favourited by ZMD    
    bool _upgradeMode;  // Resolver has been called with doUpgrade

    bool _preferHighestVersion; // Prefer the result with the newest version
                               //if there are more solver results. 
    
    // In order reducing solver time we are reducing the branches
    // by skipping resolvables which have worse architecture,edition
    // than a resolvable which provides the same cababilities.
    // BUT if there is no valid solution we will regard the "other"
    // resolvables in a second solver run too.
    bool _tryAllPossibilities; // Try ALL alternatives
    bool _skippedPossibilities;// Flag that there are other possibilities
                               // which we are currently ignore
    
  public:
    ResolverContext (const ResPool & pool, const Arch & arch, ResolverContext_Ptr parent = NULL);
    virtual ~ResolverContext();

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ResolverContext & context);

    // ---------------------------------- accessors

    unsigned long long downloadSize(void) const { return _download_size; }
    unsigned long long installSize(void) const { return _install_size; }
    int totalPriority (void) const { return _total_priority; }
    int minPriority (void) const { return _min_priority; }
    int maxPriority (void) const { return _max_priority; }
    int otherPenalties (void) const { return _other_penalties; }

    bool isValid (void) const { return !_invalid; }
    bool askUser (void) const { return _askUser; }
    bool isInvalid (void) const { return _invalid; }

    bool verifying (void) const { return _verifying; }
    void setVerifying (bool verifying) { _verifying = verifying; }

    bool tryAllPossibilities (void) const { return _tryAllPossibilities; }
    void setTryAllPossibilities (bool tryAllPossibilities) { _tryAllPossibilities = tryAllPossibilities; }
    
    bool skippedPossibilities (void) const { return _skippedPossibilities; }
    void setSkippedPossibilities (bool skippedPossibilities) { _skippedPossibilities = skippedPossibilities; }

    bool establishing (void) const { return _establishing; }
    void setEstablishing (bool establishing) { _establishing = establishing; }

    inline ResPool pool() const { return _pool; }

    inline Arch architecture() const { return _architecture; }

    // ---------------------------------- ignore capabilities
    void setIgnoreCababilities(const IgnoreMap ignoreConflicts,
			       const IgnoreMap ignoreRequires,
			       const IgnoreMap ignoreObsoletes,
			       const PoolItemList ignoreInstalledItem,
			       const PoolItemList ignoreArchitectureItem)
	{_ignoreConflicts = ignoreConflicts;
	_ignoreRequires = ignoreRequires;
	_ignoreObsoletes = ignoreObsoletes;
	_ignoreInstalledItem = ignoreInstalledItem;
	_ignoreArchitectureItem = ignoreArchitectureItem;}

    const IgnoreMap getIgnoreConflicts() const { return _ignoreConflicts; }
    const IgnoreMap getIgnoreRequires() const { return _ignoreRequires; }
    const IgnoreMap getIgnoreObsoletes() const { return _ignoreObsoletes; }    
    const PoolItemList getIgnoreInstalledItem() const { return _ignoreInstalledItem; }
    const PoolItemList getIgnoreArchitectureItem() const { return _ignoreArchitectureItem; }

    
    void setForceResolve (const bool force) { _forceResolve = force; }
    const bool forceResolve() { return _forceResolve; }

    void setEstablishContext (const ResolverContext_Ptr establish_context) { _establish_context = establish_context; }

    void setPreferHighestVersion (const bool highestVersion) { _preferHighestVersion = highestVersion; }
    const bool preferHighestVersion() { return _preferHighestVersion; }  

    void setUpgradeMode (const bool upgrade) { _upgradeMode = upgrade; }
    const bool upgradeMode() { return _upgradeMode; }

    void setUserDeleteItems ( const PoolItemList & deleteItems) { _userDeleteItems = deleteItems; }
    void setUserInstallItems ( const PoolItemList& installItems) { _userInstallItems = installItems; }
    void setUserLockUninstalledItems ( const PoolItemList& lockItems) { _userLockUninstalledItems = lockItems; }
    PoolItemList userDeleteItems () { return _userDeleteItems; }
    PoolItemList userInstallItems () { return _userInstallItems; }
    PoolItemList userLockUninstalledItems () { return _userLockUninstalledItems; }
    
    // ---------------------------------- methods

    /**
     *get the state of \c item
     *This is NOT the status in the pool but the status according
     * to the context. */
    ResStatus getStatus (PoolItem_Ref item);

    /** state change functions
    *  they do some checking before actually changing the state of the PoolItem. */

    /**
     *set the state of \c item to \c status
     *If \c status is not the current state of \c item, make \c item
     *part of the current transaction (the context) */
    void setStatus (PoolItem_Ref item, const ResStatus & status);

    /**
     *set \c item to \a to-be-installed */
    bool install (PoolItem_Ref item, bool is_soft, int other_penalty);

    /**
     *set \c item to \a satisfied */
    bool satisfy (PoolItem_Ref item, int other_penalty);

    /**
     *set \c item to \a unneeded */
    bool unneeded (PoolItem_Ref item, int other_penalty);

    /**
     *set \c item to \a incomplete */
    bool incomplete (PoolItem_Ref item, int other_penalty);

    /**
     *upgrade \c from to \c to
     *marks \c from as \a to-be-uninstalled and \c to as \a to-be-installed  */
    bool upgrade (PoolItem_Ref to, PoolItem_Ref from, bool is_soft, int other_penalty);

    /**
     *set \c item to \a to-be-uninstalled */
    bool uninstall (PoolItem_Ref item, bool part_of_upgrade, bool due_to_obsolete, bool due_to_unlink);

    // rough installed/uninstalled test for 'after transaction'

    /**
     *\return \c true if \c item is \a installed or \a to-be-installed */
    bool isPresent (PoolItem_Ref item, bool *unneeded = NULL,
		    bool *installed = NULL);

    /**
     *\return \c true if \c item is \a uninstalled or \a to-be-uninstalled */
    bool isAbsent (PoolItem_Ref item);

    bool requirementIsMet (const Capability & cap,
			   const PoolItem_Ref who,
			   const Dep & capKind,
			   bool *unneeded = NULL,
			   bool *installed = NULL);
    /**
     *\return \c true if the requirement is already fulfilled.
     *either by an installed item or the requirement is unneeded.
     *The behaviour depends on the item kind (package,patch,..)
     *which requires this capability.
     */
    bool requirementIsInstalledOrUnneeded (const Capability & capability,
					   const PoolItem_Ref who,
					   const Dep & capKind);
    bool requirementIsPossible (const Capability & cap);
    bool itemIsPossible (const PoolItem_Ref item);
    bool isParallelInstall (const PoolItem_Ref item) const;
    PoolItem_Ref getParallelInstall (const PoolItem_Ref item) const;

    /** iterate over various states */

    void foreachMarked (MarkedPoolItemFn fn, void *data) const;
    PoolItemList getMarked (int which);					// <0:uninstalls, 0:all; >0:installs
    
    int foreachInstall (MarkedPoolItemFn fn, void *data) const;
    PoolItemList getInstalls (void) const;
    int installCount (void) const;

    int foreachUninstall (MarkedPoolItemFn fn, void *data);
    PoolItemList getUninstalls (void);
    int uninstallCount (void);

    int foreachUpgrade (MarkedPoolItemPairFn fn, void *data);
    PoolItemList getUpgrades (void);
    int upgradeCount (void);

    int foreachSatisfy (MarkedPoolItemFn fn, void *data) const;
    PoolItemList getSatisfies (void) const;
    int satisfyCount (void) const;

    int foreachIncomplete (MarkedPoolItemFn fn, void *data) const;
    PoolItemList getIncompletes (void) const;
    int incompleteCount (void) const;

    int foreachImpossible (MarkedPoolItemFn fn, void *data);
//    PoolItemList getImpossibles (void);
//    int impossibleCount (void);

    // add to the report log
    void addInfo (ResolverInfo_Ptr info, bool askUser = false);	// normal progress info
    void addError (ResolverInfo_Ptr info, bool askUser = false);// error progress info
    
    // iterate over report log
    void foreachInfo (PoolItem_Ref item, int priority, ResolverInfoFn fn, void *data) const;
    ResolverInfoList getInfo (void) const;

    // Context compare to identify equal branches
    void collectCompareInfo (int & cmpVersion,    // Version compare of ACCUMULATED items
			     int & cmpSource,    // compare of Sources
			     ResolverContext_Ptr compareContext);
    
    int partialCompare (ResolverContext_Ptr context);
    int compare (ResolverContext_Ptr context);

    // debug
    void spew (void);
    void spewInfo (void) const;

    int getSourcePriority (Source_Ref source) const;
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

