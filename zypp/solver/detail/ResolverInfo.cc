/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfo.cc
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

#include <map>
#include <sstream>

#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/Repository.h"
#include "zypp/Capability.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Logger.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(ResolverInfo);

//---------------------------------------------------------------------------

#define RIT(x) RESOLVER_INFO_TYPE_ ## x, #x
static struct {
    ResolverInfoType type;
    const char	*typestr;
    const char  *str;
} type_str_table[] = {
    { RIT(NEEDED_BY),			"needed_by" },
    { RIT(CONFLICTS_WITH),		"conflicts_with" },
    { RIT(OBSOLETES),			"obsoletes" },
    { RIT(DEPENDS_ON),			"depended_on" },
    { RIT(CHILD_OF),			"child_of" },
    { RIT(MISSING_REQ),			"missing_req" },

    { RIT(INVALID_SOLUTION),		"Marking this resolution attempt as invalid" },
    { RIT(UNINSTALLABLE),		"Marking p as uninstallable" },
    { RIT(REJECT_INSTALL),		"p is scheduled to be installed, but this is not possible because of dependency problems." },
    { RIT(INSTALL_TO_BE_UNINSTALLED),	"Can't install p since it is already marked as needing to be uninstalled" },
    { RIT(INSTALL_UNNEEDED),		"Can't install p since it is does not apply to this system." },
    { RIT(INSTALL_PARALLEL),		"Can't install p, since a resolvable of the same name is already marked as needing to be installed." },
    { RIT(INCOMPLETES),			"This would invalidate p" },
	// from QueueItemEstablish
    { RIT(ESTABLISHING),		"Establishing p" },
	// from QueueItemInstall
    { RIT(INSTALLING),			"Installing p" },
    { RIT(UPDATING),			"Updating p" },
    { RIT(SKIPPING),			"Skipping p, already installed" },
	// from QueueItemRequire
    { RIT(NO_OTHER_PROVIDER),		"There are no alternative installed providers of c [for p]" },
    { RIT(NO_PROVIDER),			"There are no installable providers of c [for p]" },
    { RIT(NO_UPGRADE),			"Upgrade to q to avoid removing p is not possible." },
    { RIT(UNINSTALL_PROVIDER),		"p provides c but is scheduled to be uninstalled" },
    { RIT(KEEP_PROVIDER),		"p provides c but is scheduled to be kept" },    
    { RIT(PARALLEL_PROVIDER),		"p provides c but another version is already installed" },
    { RIT(NOT_INSTALLABLE_PROVIDER),	"p provides c but is uninstallable" },
    { RIT(LOCKED_PROVIDER),		"p provides c but is locked" },
    { RIT(OTHER_ARCH_PROVIDER),		"p provides c but has other architecture" },
    { RIT(CANT_SATISFY),		"Can't satisfy requirement c" },
	// from QueueItemUninstall
    { RIT(UNINSTALL_TO_BE_INSTALLED),	"p is to-be-installed, so it won't be unlinked." },
    { RIT(UNINSTALL_INSTALLED),		"p is required by installed, so it won't be unlinked." },
    { RIT(UNINSTALL_LOCKED),		"cant uninstall, its locked" },
	// from QueueItemConflict
    { RIT(CONFLICT_CANT_INSTALL),	"to-be-installed p conflicts with q due to c" },
    { RIT(CONFLICT_UNINSTALLABLE),	"uninstalled p is marked uninstallable it conflicts [with q] due to c" },
    { RIT(INVALID),			"invalid" },
    { RIT(INVALID), NULL }
};
#undef RIT

static const char *
info_type_to_string (ResolverInfoType type)
{
    int i;

    for (i = 0; type_str_table[i].str != NULL; ++i) {
	if (type == type_str_table[i].type)
	    return type_str_table[i].typestr;
    }

    return "?ResolverInfoType?";
}


static const char *
info_type_to_message (ResolverInfoType type)
{
    int i;

    for (i = 0; type_str_table[i].str != NULL; ++i) {
	if (type == type_str_table[i].type)
	    return type_str_table[i].str;
    }

    return "?ResolverInfoType?";
}


ResolverInfoType
resolver_info_type_from_string (const char *str)
{
    int i;

    if (str == NULL) return RESOLVER_INFO_TYPE_INVALID;

    for (i = 0; type_str_table[i].str != NULL; ++i) {
	if (strcasecmp (str, type_str_table[i].str) == 0)
	    return type_str_table[i].type;
    }

    return RESOLVER_INFO_TYPE_INVALID;
}

string
ResolverInfo::toString (PoolItem_Ref item, bool shortVersion)
{
    ostringstream os;
    if (!item) return "";

    if (item->kind() != ResTraits<zypp::Package>::kind)
	os << item->kind() << ':';
    os  << item->name();
    if (!shortVersion) {
	os << '-' << item->edition();
	if (item->arch() != "") {
	    os << '.' << item->arch();
	}
	Repository s = item->repository();
	if (s) {
	    string alias = s.info().alias();
	    if (!alias.empty()
		&& alias != "@system")
	    {
		os << '[' << s.info().alias() << ']';
	    }
	}
    }
    return os.str();
}


string
ResolverInfo::toString (const Capability & capability)
{
    ostringstream os;
    os << capability.asString();
    return os.str();
}

string
ResolverInfo::message( ) const
{
    return string(info_type_to_message(_type)) + " " + toString(_affected);
}
//---------------------------------------------------------------------------

std::ostream &
ResolverInfo::dumpOn( std::ostream & os ) const
{
    os << "ResolverInfo<";
    os << info_type_to_string (_type);
    os << "> ";

    if (_affected) {
	os << toString (_affected);
    }

    if (_error) os << " " << _(" Error!");
    if (_important) os << " " << _(" Important!");

    return os;
}

//---------------------------------------------------------------------------

ResolverInfo::ResolverInfo (ResolverInfoType type, PoolItem_Ref item, int priority)
    : _type (type)
    , _affected (item)
    , _priority (priority)
    , _error (false)
    , _important (false)
{
    _XDEBUG(*this);
}


ResolverInfo::~ResolverInfo()
{
}

//---------------------------------------------------------------------------

bool
ResolverInfo::merge (ResolverInfo_Ptr to_be_merged)
{
    if (to_be_merged == NULL) return false;

    if (_type != to_be_merged->_type
	|| _affected != to_be_merged->_affected
	|| _error != to_be_merged->_error) {
	return false;
    }

    return true;
}

void
ResolverInfo::copy (ResolverInfo_constPtr from)
{
    _error = from->_error;
    _important = from->_important;
}


ResolverInfo_Ptr
ResolverInfo::copy (void) const
{
    ResolverInfo_Ptr cpy = new ResolverInfo(_type, _affected, _priority);

    cpy->copy (this);
 
    return cpy;
}


//---------------------------------------------------------------------------

bool
ResolverInfo::isAbout (PoolItem_Ref item) const
{
    if (!_affected)
	return false;

    return _affected->name() == item->name();
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

