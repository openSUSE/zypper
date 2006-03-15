/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/PatchContentsImpl.cc
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */

#include "zypp/ui/PatchContentsImpl.h"
#include <zypp/ZYppFactory.h>
#include <zypp/ResPool.h>
#include <zypp/Atom.h>

using std::endl;

namespace zypp
{
    namespace ui
    {
	PatchContents::Impl::Impl( Patch::constPtr patch )
	    : _patch( patch )
	{
	    Patch::AtomList atomList = _patch->atoms();
	    _contents.reserve( atomList.size() );

	    ResPool pool = getZYpp()->pool();

	    for( Patch::AtomList::iterator atom_it = atomList.begin();
		 atom_it != atomList.end();
		 ++atom_it )
	    {
		ResPool::byName_iterator foundNames    = pool.byNameBegin( (*atom_it)->name() );
		ResPool::byName_iterator foundNamesEnd = pool.byNameEnd  ( (*atom_it)->name() );

		bool found = false;

		while ( ! found
			&& foundNames != foundNamesEnd )
		{
		    // DBG << "Found " << (*atom_it)->name() << " " << (*foundNames)->edition().asString() << endl;

		    found = (*atom_it)->edition() == (*foundNames)->edition();

		    if ( found )
		    {
			_contents.push_back( *foundNames );
			// DBG << "Found the right one: " << (*foundNames)->name() << " " << (*foundNames)->edition() << endl;
		    }
		    else
			++foundNames;
		}

		if ( ! found )
		{
		    WAR << "Can't find resolvable with name " << (*atom_it)->name() << " in pool" << endl;
		}
	    }
	}


	PatchContents::const_iterator PatchContents::Impl::begin() const
	{
	    return _contents.begin();
	}


	PatchContents::const_iterator PatchContents::Impl::end() const
	{
	    return _contents.end();
	}


	bool PatchContents::Impl::empty() const
	{
	    return _contents.empty();
	}


	PatchContents::size_type PatchContents::Impl::size() const
	{
	    return _contents.size();
	}

    } // namespace ui
} // namespace zypp
