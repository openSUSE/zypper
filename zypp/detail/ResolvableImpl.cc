/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ResolvableImpl.cc
 *
*/
#include <iostream>

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"

#include "zypp/base/Algorithm.h"
#include "zypp/base/Logger.h"
#include "zypp/detail/ResolvableImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    struct FilterUnwantedReq
    {
      bool operator()( const Capability & cap_r ) const
      {
        return cap_r.index().substr( 0, 7 ) == "rpmlib(";
      }
    };

    void filterUnwantedReq( const CapSet & from, CapSet & to )
    {
      to.clear();
      std::remove_copy_if( from.begin(), from.end(),
                           std::inserter( to, to.end() ),
                           FilterUnwantedReq() );
    }
  }

  namespace
  {
    struct FilterLocaleProvides
    {
      Dependencies & deps;
      ZYpp::Ptr _zypp;

      FilterLocaleProvides( Dependencies & d, ZYpp::Ptr z )
	: deps( d )
	, _zypp( z )
      { }

      bool operator()( const Capability & cap_r ) const
      {
	if (cap_r.index().substr( 0, 7 ) != "locale(")
	    return false;

	CapFactory f;

	string provides( cap_r.index(), 7 );			// strip "locale("
	string::size_type pos = provides.find( ":" );		// colon given ?
	if (pos != string::npos) {
	    deps[Dep::SUPPLEMENTS].insert( f.parse( ResTraits<Package>::kind, string( provides, 0, pos ) ) );
	    provides.erase( 0, pos+1 );
	}
	pos = 0;
	string::size_type next = pos;
	while (pos < provides.size()) {
	    next = provides.find( ";", pos );			// look for ; separator
	    if (next == string::npos)
		next = provides.size()-1;			// none left, set next to end-1 (strip trailing ')' )

	    string loc( provides, pos, next-pos );
	    if (loc.size() < 2
		|| loc.size() > 5)
	    {
		WAR << "Looks wrong " << cap_r.index() << endl;
	    }
	    if (_zypp) _zypp->availableLocale( Locale( loc ) );
	    deps[Dep::FRESHENS].insert( f.parse( ResTraits<Language>::kind, loc ) );
	    pos = next + 1;
	}
	return true;
      }
    };

    void filterLocaleProvides( const Dependencies & from, Dependencies & to, ZYpp::Ptr z )
    {

      CapSet provides;
      FilterLocaleProvides flp( to, z );

      std::remove_copy_if( from[Dep::PROVIDES].begin(), from[Dep::PROVIDES].end(),
                           std::inserter( provides, provides.end() ),
                           flp );
      to[Dep::PROVIDES] = provides;
    }
  }

  // static backref to ZYpp to inject available locales
  ZYpp::Ptr Resolvable::Impl::_zypp;

  Resolvable::Impl::Impl( const Kind & kind_r,
                          const NVRAD & nvrad_r )
  : _kind( kind_r )
  , _name( nvrad_r.name )
  , _edition( nvrad_r.edition )
  , _arch( nvrad_r.arch )
  , _deps( nvrad_r )
  {
    if (!_zypp) _zypp = zypp::getZYpp();

    // check if we provide any 'locale(...)' tags and split them
    //  up to freshens/supplements

    filterLocaleProvides( nvrad_r, _deps, _zypp );

    // assert self provides
    _deps[Dep::PROVIDES].insert( CapFactory()
                                 .parse( _kind, _name, Rel::EQ, _edition ) );

    // Filter 'rpmlib(...)' requirements (refill from nvrad_r)
    filterUnwantedReq( nvrad_r[Dep::PREREQUIRES], _deps[Dep::PREREQUIRES] );
    filterUnwantedReq( nvrad_r[Dep::REQUIRES], _deps[Dep::REQUIRES] );

    // assert all prerequires are in requires too
    _deps[Dep::REQUIRES].insert( _deps[Dep::PREREQUIRES].begin(),
                                 _deps[Dep::PREREQUIRES].end() );

  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
