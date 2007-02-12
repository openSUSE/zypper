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
#include <string>
#include <map>
#include "zypp/base/Logger.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"

#include "zypp/base/Algorithm.h"
#include "zypp/base/Sysconfig.h"
#include "zypp/detail/ResolvableImpl.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/capability/Capabilities.h"

using std::endl;

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
    struct FilterExtraDependency
    {
      Dependencies & deps;
	CapSet & languages;

      FilterExtraDependency( Dependencies & d , CapSet  & l)
	  : deps( d ),
	    languages( l )
      { }

      bool operator()( const Capability & cap_r ) const
      {
	if ( isKind<capability::ModaliasCap>(cap_r) )
          {
            // in case cap provides a packagename, inject a SUPPLEMENTS.
	    // if modalias does not provide a packagename, default to "kernel" (#184840)

            intrusive_ptr<const capability::ModaliasCap> cap( capability::asKind<capability::ModaliasCap>(cap_r) );
            if ( cap ) {
	      std::string pkgname( cap->pkgname() );
	      if ( pkgname.empty() ) {
		pkgname = "kernel";		// every kernel provides "kernel", so this triggers always
	      }
              deps[Dep::SUPPLEMENTS].insert( CapFactory().parse( ResTraits<Package>::kind, pkgname ) );
              deps[Dep::FRESHENS].insert(cap_r);
	    }
            return true;	// strip from original deps, we just splitted it to supplements/freshens
          }

	if ( isKind<capability::HalCap>(cap_r) )
          {
	    deps[Dep::SUPPLEMENTS].insert( cap_r );
            return true;	// strip from provides
          }

	if (cap_r.index().substr( 0, 7 ) != "locale(")
	    return false;

	CapFactory f;

	std::string locales( cap_r.index(), 7 );			// strip "locale("
	std::string::size_type pos = locales.find( ":" );		// colon given ?
	if (pos != std::string::npos) {
	    deps[Dep::SUPPLEMENTS].insert( f.parse( ResTraits<Package>::kind, std::string( locales, 0, pos ) ) );
	    locales.erase( 0, pos+1 );
	}
	pos = 0;
	std::string::size_type next = pos;
	while (pos < locales.size()) {
	    next = locales.find( ";", pos );			// look for ; separator
	    if (next == std::string::npos)
		next = locales.size()-1;			// none left, set next to end-1 (strip trailing ')' )

	    std::string loc( locales, pos, next-pos );
	    getZYpp()->availableLocale( Locale( loc ) );
	    deps[Dep::FRESHENS].insert( f.parse( ResTraits<Language>::kind, loc ) );
	    languages.insert( f.parse( ResTraits<Language>::kind, loc ) );
	    pos = next + 1;
	}
	return true;
      }
    };

    void filterExtraProvides( const Dependencies & from, Dependencies & to )
    {
      CapSet provides;
      CapSet languages;
      
      FilterExtraDependency flp( to, languages );

      std::remove_copy_if( from[Dep::PROVIDES].begin(), from[Dep::PROVIDES].end(),
                           std::inserter( provides, provides.end() ),
                           flp );
      to[Dep::PROVIDES] = provides;

      // There are language dependencies without a triggering package (e.G. locale(de) ).
      // So if there is no supplement, the language will be inserted in the supplements too.
      // (Not only in the freshens). Bug 178721 and 240617
      if (languages.size() > 0
	  && to[Dep::SUPPLEMENTS].size() == 0) {
	  to[Dep::SUPPLEMENTS] = languages;
      }
    }

    void filterExtraSupplements( const Dependencies & from, Dependencies & to )
    {
      CapSet supplements;
      CapSet dummy;      
      to[Dep::SUPPLEMENTS].clear(); 

      FilterExtraDependency flp( to, dummy );

      std::remove_copy_if( from[Dep::SUPPLEMENTS].begin(), from[Dep::SUPPLEMENTS].end(),
                           std::inserter( supplements, supplements.end() ),
                           flp );
      to[Dep::SUPPLEMENTS].insert(supplements.begin(), supplements.end());
    }

    // rewrite dependencies from
    //   kernel(xxx) = yyy
    // to
    //   kernel($FLAVOR:xxx) = yyy
    // $flavor is determined by searching through
    //   PROVIDES (for kernel-$flavor, by kernel package)
    // or
    //   REQUIRES (for kernel-$flavor, by kmp package)
    // see bugzilla #190163
    //

    std::string findKernelFlavor( const CapSet & cset, const Dep & dep )
    {
      for (CapSet::iterator it = cset.begin(); it != cset.end(); ++it) {

	// check for "kernel-" in deps
	// if its a requires, take it as is
	// if its a provides, check for non-empty edition since
	//   kernels provide "kernel-flavor-nongpl" (empty edition)
	//     and "kernel-flavor = x.y" (non-empty edition)

	if ( it->index().substr( 0, 7 ) == "kernel-"
	     && (dep == Dep::REQUIRES
		|| it->edition() != Edition::noedition ) )
	{
	  return it->index().erase( 0, 7 );	// erase "kernel-"
	}
      }
      return "";
    }

    void rewriteKernelDeps( Dependencies & deps )
    {
      // check the smaller set (requires) first
      Dep dep = Dep::REQUIRES;
      CapSet cset = deps[dep];
      std::string flavor( findKernelFlavor( cset, dep ) );
      if (flavor.empty()) {
	dep = Dep::PROVIDES;
	cset = deps[dep];
	flavor = findKernelFlavor( cset, dep );
      }

      if (flavor.empty())		// not a kernel / kmp package
	return;

      // flavor == kernel flavor
      // cset == CapSet to be rewritten (provides for kernel, requires for kmp)
      // dep == deps to be set in 'to'

      flavor.append( ":" );
      CapFactory factory;
      deps[dep].clear();
      for (CapSet::iterator it = cset.begin(); it != cset.end(); ++it) {
	std::string idx( it->index() );
	if ( idx.substr( 0, 7 ) == "kernel("		// capability is "kernel(..."
	     && idx.find( ":" ) == std::string::npos )	//  without a colon
	{
	  deps[dep].insert( factory.parse( it->refers(), idx.insert( 7, flavor ), it->op(), it->edition() ) );
	}
	else {
	  deps[dep].insert( *it );
	}
      }
      return;
    }

  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Resolvable::Impl::Impl
  //	METHOD TYPE : Ctor
  //
  Resolvable::Impl::Impl( const Kind & kind_r,
                          const NVRAD & nvrad_r )
  : _kind( kind_r )
  , _name( nvrad_r.name )
  , _edition( nvrad_r.edition )
  , _arch( nvrad_r.arch )
  , _deps( nvrad_r )
  {
    // check if we provide/supplements any extra ('locale(...)', 'modalias(...)', ...) tags
    //  and split them up to freshens/supplements (except for SystemResObject)
    if ( _kind != ResTraits<SystemResObject>::kind )
      {
        filterExtraSupplements( nvrad_r, _deps );
        filterExtraProvides( nvrad_r, _deps );
      }

    // remove malicious self provides
    CapSet::iterator it = _deps[Dep::PROVIDES].find( CapFactory().parse( _kind, _name ) );
      if ( it != _deps[Dep::PROVIDES].end() )
        {
          dumpOn( WAR << "Strip self provides without edition in " ) << endl;
          _deps[Dep::PROVIDES].erase( it );
        }

    // assert self provides
    _deps[Dep::PROVIDES].insert( CapFactory()
                                 .parse( _kind, _name, Rel::EQ, _edition ) );

    // Filter 'rpmlib(...)' requirements (refill from nvrad_r)
    filterUnwantedReq( nvrad_r[Dep::PREREQUIRES], _deps[Dep::PREREQUIRES] );
    filterUnwantedReq( nvrad_r[Dep::REQUIRES], _deps[Dep::REQUIRES] );

    // check for kernel(xxx) and rewrite them to kernel(flavor:xxx)
    if ( _kind == ResTraits<Package>::kind )
      {
	rewriteKernelDeps( _deps );   
      }

    // assert all prerequires are in requires too
    _deps[Dep::REQUIRES].insert( _deps[Dep::PREREQUIRES].begin(),
                                 _deps[Dep::PREREQUIRES].end() );

    if ( _arch.empty() )
      dumpOn( WAR << "Has empty Arch: " ) << std::endl;
  }

  std::ostream & Resolvable::Impl::dumpOn( std::ostream & str ) const
  {
    return str << '[' << kind() << ']'
               << name() << '-' << edition() << '.' << arch();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
