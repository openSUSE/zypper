#/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfo.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVER_INFO_H
#define ZYPP_SOLVER_DETAIL_RESOLVER_INFO_H

#include "zypp/solver/detail/Types.h"

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
    RESOLVER_INFO_TYPE_INVALID = 0,
    RESOLVER_INFO_TYPE_NEEDED_BY,
    RESOLVER_INFO_TYPE_CONFLICTS_WITH,
    RESOLVER_INFO_TYPE_OBSOLETES,
    RESOLVER_INFO_TYPE_DEPENDS_ON,
    RESOLVER_INFO_TYPE_CHILD_OF,
    RESOLVER_INFO_TYPE_MISSING_REQ,
	// from ResolverContext
    RESOLVER_INFO_TYPE_INVALID_SOLUTION,		// Marking this resolution attempt as invalid.
    RESOLVER_INFO_TYPE_UNINSTALLABLE,			// Marking p as uninstallable
    RESOLVER_INFO_TYPE_REJECT_INSTALL,			// p is scheduled to be installed, but this is not possible because of dependency problems.
    RESOLVER_INFO_TYPE_INSTALL_TO_BE_UNINSTALLED,	// Can't install p since it is already marked as needing to be uninstalled
    RESOLVER_INFO_TYPE_INSTALL_UNNEEDED,		// Can't install p since it is does not apply to this system.
    RESOLVER_INFO_TYPE_INSTALL_PARALLEL,		// Can't install p, since a resolvable of the same name is already marked as needing to be installed.
    RESOLVER_INFO_TYPE_INCOMPLETES,			// This would invalidate p
	// from QueueItemEstablish
    RESOLVER_INFO_TYPE_ESTABLISHING,			// Establishing p
	// from QueueItemInstall
    RESOLVER_INFO_TYPE_INSTALLING,			// Installing p
    RESOLVER_INFO_TYPE_UPDATING,			// Updating p
    RESOLVER_INFO_TYPE_SKIPPING,			// Skipping p, already installed
	// from QueueItemRequire
    RESOLVER_INFO_TYPE_NO_OTHER_PROVIDER,		// There are no alternative installed providers of c [for p]
    RESOLVER_INFO_TYPE_NO_PROVIDER,			// There are no installable providers of c [for p]
    RESOLVER_INFO_TYPE_NO_UPGRADE,			// Upgrade to q to avoid removing p is not possible.
    RESOLVER_INFO_TYPE_UNINSTALL_PROVIDER,		// p provides c but is scheduled to be uninstalled
    RESOLVER_INFO_TYPE_PARALLEL_PROVIDER,		// p provides c but another version is already installed
    RESOLVER_INFO_TYPE_NOT_INSTALLABLE_PROVIDER,	// p provides c but is uninstallable
    RESOLVER_INFO_TYPE_LOCKED_PROVIDER,			// p provides c but is locked
    RESOLVER_INFO_TYPE_CANT_SATISFY,			// Can't satisfy requirement c
	// from QueueItemUninstall
    RESOLVER_INFO_TYPE_UNINSTALL_TO_BE_INSTALLED,	// p is to-be-installed, so it won't be unlinked.
    RESOLVER_INFO_TYPE_UNINSTALL_INSTALLED,		// p is required by installed, so it won't be unlinked.
    RESOLVER_INFO_TYPE_UNINSTALL_LOCKED,		// cant uninstall, its locked
	// from QueueItemConflict
    RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL,		// to-be-installed p conflicts with q due to c
    RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE		// uninstalled p is marked uninstallable it conflicts [with q] due to c
} ResolverInfoType;

#define RESOLVER_INFO_PRIORITY_USER      500
#define RESOLVER_INFO_PRIORITY_VERBOSE   100
#define RESOLVER_INFO_PRIORITY_DEBUGGING   0

typedef void (*ResolverInfoFn) (ResolverInfo_Ptr info, void *data);

typedef std::list <ResolverInfo_Ptr> ResolverInfoList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfo

class ResolverInfo : public base::ReferenceCounted, private base::NonCopyable {

  private:

    ResolverInfoType _type;

    PoolItem_Ref _affected;

    int _priority;

    bool _error;
    bool _important;

  protected:

    ResolverInfo (ResolverInfoType type, PoolItem_Ref affected, int priority);

  public:

    virtual ~ResolverInfo();

    void copy (ResolverInfo_constPtr from);

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ResolverInfo & context);

    // ---------------------------------- accessors

    ResolverInfoType type (void) const { return _type; }
    PoolItem_Ref affected (void) const { return _affected; }
    int priority (void) const { return _priority; }

    int error (void) const { return _error; }
    void flagAsError (void) { _error = true; }
    int important (void) const { return _important; }
    void flagAsImportant (void) { _important = true; }

    // ---------------------------------- methods

    bool merge (ResolverInfo_Ptr to_be_merged);
    virtual ResolverInfo_Ptr copy (void) const;

    bool isAbout (PoolItem_Ref item) const;
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
#endif // ZYPP_SOLVER_DETAIL_RESOLVER_INFO_H

