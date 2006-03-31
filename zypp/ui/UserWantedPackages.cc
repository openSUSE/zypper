/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file	zypp/ui/UserWantedPackages.cc
 *
 *  \author	Stefan Hundhammer <sh@suse.de>
 *
 */

#include "zypp/ui/UserWantedPackages.h"

#include "zypp/ui/Status.h"
#include "zypp/ui/Selectable.h"
#include "zypp/ResObject.h"
#include "zypp/Package.h"
#include "zypp/Selection.h"
#include "zypp/Pattern.h"
#include "zypp/Language.h"
#include "zypp/Patch.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"


using std::string;
using std::set;
using std::endl;


namespace zypp
{
    namespace ui
    {
	typedef ResPoolProxy::const_iterator	PoolProxyIterator;

        static inline ResPoolProxy		poolProxy()	{ return getZYpp()->poolProxy();	}

	template<class T> PoolProxyIterator poolProxyBegin()	{ return poolProxy().byKindBegin<T>();	}
	template<class T> PoolProxyIterator poolProxyEnd()	{ return poolProxy().byKindEnd<T>();	}

	static inline PoolProxyIterator pkgBegin()		{ return poolProxyBegin<Package>();	}
	static inline PoolProxyIterator pkgEnd()		{ return poolProxyEnd<Package>();	}

	static inline PoolProxyIterator langBegin()		{ return poolProxyBegin<Language>();	}
	static inline PoolProxyIterator langEnd()		{ return poolProxyEnd<Language>();	}

	static inline PoolProxyIterator patchesBegin()		{ return poolProxyBegin<Patch>();	}
	static inline PoolProxyIterator patchesEnd()		{ return poolProxyEnd<Patch>();		}

	template<typename T> bool contains( const std::set<T> & container, T search )
	{
	    return container.find( search ) != container.end();
	}



	static void addDirectlySelectedPackages	( set<string> & pkgNames );
        template<class PkgSet_T> void addPkgSetPackages( set<string> & pkgNames );

	static void addSelectionPackages	( set<string> & pkgNames );
	static void addPatternPackages		( set<string> & pkgNames );
	static void addLanguagePackages		( set<string> & pkgNames );
	static void addPatchPackages		( set<string> & pkgNames );



	set<string> userWantedPackageNames()
	{
	    set<string> pkgNames;

	    DBG << "Collecting packages the user explicitly asked for" << endl;
	    
	    addDirectlySelectedPackages	( pkgNames );
	    addSelectionPackages	( pkgNames );
	    addPatternPackages		( pkgNames );
	    addLanguagePackages		( pkgNames );
	    addPatchPackages		( pkgNames );

	    return pkgNames;
	}



	static void addDirectlySelectedPackages( set<string> & pkgNames )
	{
	    for ( PoolProxyIterator it = pkgBegin();
		  it != pkgEnd();
		  ++it )
	    {
		// Add all packages the user wanted to transact directly,
		// no matter what the transaction is (install, update, delete)

		if ( (*it)->toModify() && (*it)->modifiedBy() == ResStatus::USER )
		{
		    DBG << "Explicit user transaction on pkg " << (*it)->theObj()->name() << endl;
		    pkgNames.insert( (*it)->theObj()->name() );
		}
	    }
	}



	static void addSelectionPackages( set<string> & pkgNames )
	{
	    addPkgSetPackages<Selection>( pkgNames );
	}


	static void addPatternPackages( set<string> & pkgNames )
	{
	    addPkgSetPackages<Pattern>( pkgNames );
	}


	/**
	 * Template to handle Selections and Patterns
	 **/
        template<class PkgSet_T> void addPkgSetPackages( set<string> & pkgNames )
	{
	    DBG << getZYpp()->pool() << endl;
		
	    for ( PoolProxyIterator pkgSet_it = poolProxyBegin<PkgSet_T>();
		  pkgSet_it != poolProxyEnd<PkgSet_T>();
		  ++pkgSet_it )
	    {
		// Take all pkg sets (selections or patterns) into account that
		// will be transacted, no matter if the user explicitly asked
		// for that pkg set or if the selection is required by another
		// pkg set of the same class

		typename PkgSet_T::constPtr pkgSet = dynamic_pointer_cast<const PkgSet_T>( (*pkgSet_it)->theObj() );

		if ( pkgSet && (*pkgSet_it)->toModify() )
		{
		    DBG << "Pattern / selection " << pkgSet->name() << " will be transacted" << endl;
		    set<string> setPkgs = pkgSet->install_packages();
		    pkgNames.insert( setPkgs.begin(), setPkgs.end() );
		}
	    }
	}



	static void addLanguagePackages( set<string> & pkgNames )
	{
	    // Build a set of all languages that are to be transacted

	    set<string> wantedLanguages;

	    for ( PoolProxyIterator lang_it = langBegin();
		  lang_it != langEnd();
		  ++lang_it )
	    {
		Language::constPtr lang = dynamic_pointer_cast<const Language>( *lang_it );

		if ( lang && (*lang_it)->toModify() )
		{
		    DBG << "Language " << lang->name() << " will be transacted" << endl;
		    wantedLanguages.insert( lang->name() );
		}
	    }


	    // Check all packages if they support any of the wanted languages

	    for ( PoolProxyIterator pkg_it = pkgBegin();
		  pkg_it != pkgEnd();
		  ++pkg_it )
	    {
		ResObject::constPtr obj = (*pkg_it)->theObj();

		if ( obj )
		{
		    CapSet freshens = obj->dep( Dep::FRESHENS );

		    for ( CapSet::const_iterator cap_it = freshens.begin();
			  cap_it != freshens.end();
			  ++cap_it )
		    {
			if ( contains( wantedLanguages, (*cap_it).index() ) )
			    pkgNames.insert( obj->name() );
		    }
		}
	    }
	}



	static void addPatchPackages( set<string> & pkgNames )
	{
	    for ( PoolProxyIterator patch_it = patchesBegin();
		  patch_it != patchesEnd();
		  ++patch_it )
	    {
		Patch::constPtr patch = dynamic_pointer_cast<const Patch>( *patch_it );

		if ( patch && (*patch_it)->toModify() )
		{
		    DBG << "Patch " << patch->name() << "(" << patch->summary() << ") will be transacted" << endl;

		    Patch::AtomList atomList = patch->atoms();

		    for ( Patch::AtomList::iterator atom_it = atomList.begin();
			  atom_it != atomList.end();
			  ++atom_it )
		    {
			pkgNames.insert( (*atom_it)->name() );
		    }
		}
	    }
	}

    }
}
