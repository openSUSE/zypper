/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/PatchContentsImpl.h
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */
#ifndef ZYPP_UI_PATCH_CONTENTS_IMPL_H
#define ZYPP_UI_PATCH_CONTENTS_IMPL_H

#include <vector>
#include "zypp/base/PtrTypes.h"
#include "zypp/ui/PatchContents.h"

namespace zypp
{
    namespace ui
    {
	class PatchContents::Impl
	{
	public:

	    Impl( Patch::constPtr patch );

	    const_iterator byKindBegin( const ResObject::Kind & kind_r ) const;
	    const_iterator byKindEnd  ( const ResObject::Kind & kind_r ) const;

	    const_iterator begin() const;
	    const_iterator end  () const;

	    bool empty() const;
	    size_type size() const;

	private:
	    Patch::constPtr	_patch;
	    ContentsCollection	_contents;
	};


    } // namespace ui
} // namespace zypp

#endif // ZYPP_UI_PATCH_CONTENTS_IMPL_H
