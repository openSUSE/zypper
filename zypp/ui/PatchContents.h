/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/PatchContents.h
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */
#ifndef ZYPP_UI_PATCH_CONTENTS_H
#define ZYPP_UI_PATCH_CONTENTS_H

#include <vector>
#include "zypp/base/PtrTypes.h"
#include "zypp/Patch.h"

namespace zypp
{
    namespace ui
    {
	/**
	 * Helper class that turns Patch::atomList() into something useful.
	 **/
	class PatchContents
	{
	protected:
	    typedef std::vector<ResObject::constPtr> ContentsCollection;

	public:
	    class Impl;

	    typedef ContentsCollection::iterator       iterator;
	    typedef ContentsCollection::const_iterator const_iterator;
	    typedef ContentsCollection::size_type      size_type;


	public:

	    PatchContents( Patch::constPtr patch );

	    const_iterator begin() const;
	    const_iterator end  () const;

	    bool empty() const;
	    size_type size() const;

	private:
	    RW_pointer<Impl> _pimpl;
	};


    } // namespace ui
} // namespace zypp

#endif // ZYPP_UI_PATCH_CONTENTS_H
