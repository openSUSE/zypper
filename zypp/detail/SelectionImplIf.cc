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
#include <iostream>

#include "zypp/detail/SelectionImplIf.h"
#include "zypp/ResTraits.h"

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

    const std::set<std::string> SelectionImplIf::install_packages( const Locale & lang) const
    {
	 std::set<std::string> result;
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
