/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoNeededBy.cc
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
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/Dep.h"

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

IMPL_PTR_TYPE(ResolverInfoNeededBy);

//---------------------------------------------------------------------------


std::ostream &
ResolverInfoNeededBy::dumpOn( std::ostream & os ) const
{
    ResolverInfo::dumpOn (os);

    ostringstream affected_str;
    affected_str << ResolverInfo::toString (affected());

    switch ( _capKind.inSwitch() )
    {
	case Dep::RECOMMENDS_e:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is recommended by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
	    break;
	case Dep::SUGGESTS_e:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is suggested by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
	    break;
	case Dep::FRESHENS_e:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is freshened by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
	    break;
	case Dep::ENHANCES_e:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is enhanced by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
	    break;
	case Dep::SUPPLEMENTS_e:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is supplemented by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
	    break;	    	    	    	    
	default:
	    // Translator: all.%s = name of package,patch,...
	    os << str::form (_("%s is needed by %s"),
			     affected_str.str().c_str(),
			     itemsToString(true).c_str());
    }
    if (_cap != Capability::noCap)
	os << " (" << _cap << ")";

    return os;
}

string
ResolverInfoNeededBy::message( ) const
{
    string affected_str = ResolverInfo::toString(affected());
    string ret;
    
    switch ( _capKind.inSwitch() )
    {
	case Dep::RECOMMENDS_e:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is recommended by %s"),
			     affected_str.c_str(),
		 	     itemsToString(false).c_str());
	    break;
	case Dep::SUGGESTS_e:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is suggested by %s"),
			     affected_str.c_str(),
			     itemsToString(false).c_str());
	    break;
	case Dep::FRESHENS_e:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is freshened by %s"),
			     affected_str.c_str(),
			     itemsToString(false).c_str());
	    break;
	case Dep::ENHANCES_e:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is enhanced by %s"),
			     affected_str.c_str(),
			     itemsToString(false).c_str());
	    break;
	case Dep::SUPPLEMENTS_e:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is supplemented by %s"),
			     affected_str.c_str(),
			     itemsToString(false).c_str());
	    break;	    	    	    	    
	default:
	    // Translator: all.%s = name of package,patch,...
	    ret = str::form (_("%s is needed by %s"),
			     affected_str.c_str(),
			     itemsToString(false).c_str());
    }
    if (_cap != Capability::noCap)
	ret += " (" + _cap.asString() + ")";

    return ret;
}

//---------------------------------------------------------------------------

ResolverInfoNeededBy::ResolverInfoNeededBy (PoolItem_Ref item)
    : ResolverInfoContainer (RESOLVER_INFO_TYPE_NEEDED_BY, item, RESOLVER_INFO_PRIORITY_USER)
    , _cap(Capability::noCap)
    , _capKind(Dep::REQUIRES)
    , _initialInstallation(false)
{
}


ResolverInfoNeededBy::~ResolverInfoNeededBy ()
{
}

//---------------------------------------------------------------------------

ResolverInfo_Ptr
ResolverInfoNeededBy::copy (void) const
{
    ResolverInfoNeededBy_Ptr cpy = new ResolverInfoNeededBy(affected());

    ((ResolverInfoContainer_Ptr)cpy)->copy (this);

    return cpy;
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

