/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatternImplIf.cc
 *
*/

#include "zypp/detail/PatternImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    bool PatternImplIf::isDefault() const
    { return false; }

    bool PatternImplIf::userVisible() const
    { return true; }

    TranslatedText PatternImplIf::category() const
    { return TranslatedText(); }

    Pathname PatternImplIf::icon() const
    { return Pathname(); }

    Pathname PatternImplIf::script() const
    { return Pathname(); }

    Label PatternImplIf::order() const
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

    std::set<std::string> PatternImplIf::install_packages( const Locale & lang) const
    {
	std::set<std::string> result;

	copycaps( result, self()->dep( Dep::REQUIRES ) );
	copycaps( result, self()->dep( Dep::RECOMMENDS) );
	copycaps( result, self()->dep( Dep::SUGGESTS) );
	
	return result;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
