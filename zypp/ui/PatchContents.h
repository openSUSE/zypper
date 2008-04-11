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
         *
         * \deprecated use Patch::contets method instead.
	 **/
	class ZYPP_DEPRECATED PatchContents
	{
	protected:
	    typedef std::vector<ResObject::constPtr> ContentsCollection;

	public:
	    class Impl;

	    typedef ContentsCollection::iterator       iterator;
	    typedef ContentsCollection::const_iterator const_iterator;
	    typedef ContentsCollection::size_type      size_type;


	public:

	    PatchContents( Patch::constPtr patch ) ZYPP_DEPRECATED;

	    const_iterator begin() const ZYPP_DEPRECATED;
	    const_iterator end  () const ZYPP_DEPRECATED;

	    bool empty() const ZYPP_DEPRECATED;
	    size_type size() const ZYPP_DEPRECATED;

	private:
	    RW_pointer<Impl> _pimpl;
	};


    } // namespace ui
} // namespace zypp

#endif // ZYPP_UI_PATCH_CONTENTS_H
