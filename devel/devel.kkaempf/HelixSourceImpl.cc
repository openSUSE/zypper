/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* HelixSourceImpl.cc
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

#include "HelixSourceImpl.h"
#include "zypp/solver/temporary/Channel.h"
#include "zypp/solver/temporary/extract.h"
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

//---------------------------------------------------------------------------

HelixSourceImpl::HelixSourceImpl(media::MediaAccess::Ptr & media_r, const Pathname & path_r)
{
    load (path_r.asString());
}


//---------------------------------------------------------------------------


static bool
add_resolvable_cb (Resolvable::Ptr res, ResStore *store)
{
#if 0
    HelixSourceImpl *undump = (HelixSourceImpl *)data;

    undump->addResItem (res);
#else
    MIL << "add_resItem_cb()" << endl;
#endif

    return true;
}


//-----------------------------------------------------------------------------

void
HelixSourceImpl::load (const string & filename)
{
    if (!filename.empty()) {
	Channel_Ptr channel = new Channel ("test", "test", "test", "test");
	channel->setType (CHANNEL_TYPE_HELIX);
//	channel->setSystem (true);

	extractHelixFile (filename, channel, add_resolvable_cb, &_store);
    }
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

