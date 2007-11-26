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

#include <iostream>
#include "zypp/base/Logger.h"
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

		    if ( isKind<Package>( (*foundNames).resolvable() ) &&
			 (*atom_it)->edition() <= (*foundNames)->edition() &&
			 (*atom_it)->arch()    == (*foundNames)->arch()      )
		    {
			found = true;
			_contents.push_back( *foundNames );

			MIL << "Found resolvable for patch atom: "
			    << (*foundNames)->name() << "-" << (*foundNames)->edition()
			    << " arch: " << (*foundNames)->arch().asString()
			    << endl;
		    }
		    else
			++foundNames;
		}

		if ( ! found )
		{
		    Arch system_arch = getZYpp()->architecture();

		    if (! (*atom_it)->arch().compatibleWith( system_arch) )
		    {
			// It's perfectly legitimate for that corresponding resolvable not to be in the pool:
			// The pool only contains resolvables in matching architectures, yet a
			// multi-arch patch might as well contain atoms for different architectures.

			continue;
		    }

		    DBG << "No resolvable for patch atom in pool: "
			<< (*atom_it)->name() << "-" << (*atom_it)->edition()
			<< " arch: " << (*atom_it)->arch().asString()
			<< endl;
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
