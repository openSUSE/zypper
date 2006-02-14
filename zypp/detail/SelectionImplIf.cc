/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImplIf.cc
 *
*/
#include "zypp/detail/SelectionImplIf.h"
#include "zypp/ResTraits.h"
#include <iostream>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // Default implementation of SelectionImplIf attributes,
    // as far as resonable.
    /////////////////////////////////////////////////////////////////

    TranslatedText SelectionImplIf::summary() const
    { return TranslatedText::notext; }

    TranslatedText SelectionImplIf::description() const
    { return TranslatedText::notext; }

    Label SelectionImplIf::category() const
    { return Label(); }

    bool SelectionImplIf::visible() const
    { return false; }

    Label SelectionImplIf::order() const
    { return Label(); }

    static void copycaps( std::set<std::string> & out, const CapSet & in)
    {
	for (CapSet::const_iterator it = in.begin(); it != in.end(); ++it) {
	    if (isKind<capability::NamedCap>( *it )
		&& it->refers() == ResTraits<zypp::Package>::kind )
	    {
		out.insert( it->index() );
	    }
	}
    }

    std::set<std::string> SelectionImplIf::install_packages( const Locale & lang) const
    {
	std::set<std::string> result;
#warning does not honor language packs
	copycaps( result, self()->dep( Dep::REQUIRES ) );
	copycaps( result, self()->dep( Dep::RECOMMENDS) );

	return result;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
