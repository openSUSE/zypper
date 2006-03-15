/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/PatchContents.cc
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */

#include "zypp/ui/PatchContentsImpl.h"

namespace zypp
{
    namespace ui
    {
	PatchContents::PatchContents( Patch::constPtr patch )
	    : _pimpl( new PatchContents::Impl( patch ) )
	{
	    // NOP
	}

	PatchContents::const_iterator PatchContents::begin() const
	{
	    return _pimpl->begin();
	}
	
	PatchContents::const_iterator PatchContents::end() const
	{
	    return _pimpl->end();
	}

	bool PatchContents::empty() const
	{
	    return _pimpl->empty();
	}

	PatchContents::size_type PatchContents::size() const
	{
	    return _pimpl->size();
	}
	
    } // namespace ui
} // namespace zypp
