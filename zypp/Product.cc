/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Product.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/Product.h"
#include "zypp/Url.h"

#include "zypp/sat/LookupAttr.h"
#include "zypp/sat/WhatProvides.h"
#include "zypp/sat/WhatObsoletes.h"
#include "zypp/PoolItem.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Product);

  namespace
  {
    void fillList( std::list<Url> & ret_r, sat::Solvable solv_r, sat::SolvAttr attr_r )
    {
      sat::LookupAttr query( attr_r, solv_r );
      for_( it, query.begin(), query.end() )
      {
        try // ignore malformed urls
        {
          ret_r.push_back( Url( it.asString() ) );
        }
        catch( const url::UrlException & )
        {}
      }
    }

    void fillList( std::list<std::string> & ret_r, sat::Solvable solv_r, sat::SolvAttr attr_r )
    {
      sat::LookupAttr query( attr_r, solv_r );
      for_( it, query.begin(), query.end() )
      {
        ret_r.push_back( it.asString() );
      }
    }
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::Product
  //	METHOD TYPE : Ctor
  //
  Product::Product( const sat::Solvable & solvable_r )
  : ResObject( solvable_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Product::~Product
  //	METHOD TYPE : Dtor
  //
  Product::~Product()
  {}

  ///////////////////////////////////////////////////////////////////

  sat::Solvable Product::referencePackage() const
  {
    Capability identCap( lookupStrAttribute( sat::SolvAttr::productReferences ) );
    if ( ! identCap )
    {
      // No 'references': fallback to provider of 'product(name) = version'
      // Without this solver testcase won't work, as it does not remember
      // 'references'.
      identCap = Capability( str::form( "product(%s) = %s", name().c_str(), edition().c_str() )  );
    }
    if ( ! identCap )
    {
      return sat::Solvable::noSolvable;
    }

    // if there is productReferences defined, we expect
    // a matching package within the same repo. And of
    // same arch.
    sat::WhatProvides providers( identCap );
    for_( it, providers.begin(), providers.end() )
    {
      if ( it->repository() == repository()
           && it->arch() == arch() )
        return *it;
    }

    WAR << *this << ": no reference package found: " << identCap << endl;
    return sat::Solvable::noSolvable;
  }

  Product::ReplacedProducts Product::replacedProducts() const
  {
    std::vector<constPtr> ret;
    // By now we simply collect what is obsoleted by the Product,
    // or by the products buddy (release-package).

    // Check our own dependencies. We should not have any,
    // but just to be shure.
    sat::WhatObsoletes obsoleting( satSolvable() );
    for_( it, obsoleting.begin(), obsoleting.end() )
    {
      if ( it->isKind( ResKind::product ) )
        ret.push_back( make<Product>( *it ) );
    }

    // If we have a buddy, we check what product buddies the
    // buddy replaces.
    obsoleting = sat::WhatObsoletes( poolItem().buddy() );
    for_( it, obsoleting.poolItemBegin(), obsoleting.poolItemEnd() )
    {
      if ( (*it).buddy().isKind( ResKind::product ) )
        ret.push_back( make<Product>( (*it).buddy() ) );
    }

    return ret;
  }

  ///////////////////////////////////////////////////////////////////

  std::string Product::shortName() const
  { return lookupStrAttribute( sat::SolvAttr::productShortlabel ); }

  std::string Product::flavor() const
  { return lookupStrAttribute( sat::SolvAttr::productFlavor ); }

  std::string Product::type() const
  { return lookupStrAttribute( sat::SolvAttr::productType ); }

  std::string Product::updaterepoKey() const
  { return lookupStrAttribute( sat::SolvAttr::productUpdaterepoKey ); }

  std::list<std::string> Product::flags() const
  {
    std::list<std::string> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productFlags );
    return ret;
  }

  std::string Product::registerTarget() const
  { return lookupStrAttribute( sat::SolvAttr::productRegisterTarget );}

  std::string Product::registerRelease() const
  { return lookupStrAttribute( sat::SolvAttr::productRegisterRelease ); }

  /////////////////////////////////////////////////////////////////

  Product::UrlList Product::urls( const std::string & key_r ) const
  {
    UrlList ret;
#warning IMPLEMENT PRODUCT URLS
    return ret;
  }

  Product::UrlList Product::releaseNotesUrls() const { return urls( "releasenotes" ); }
  Product::UrlList Product::registerUrls()     const { return urls( "register" ); }
  Product::UrlList Product::smoltUrls()        const { return urls( "smolt" ); }
  Product::UrlList Product::updateUrls()       const { return urls( "update" ); }
  Product::UrlList Product::extraUrls()        const { return urls( "extra" ); }
  Product::UrlList Product::optionalUrls()     const { return urls( "optional" ); }

  std::ostream & operator<<( std::ostream & str, const Product::UrlList & obj )
  { return dumpRange( str << obj.key() << ' ', obj.begin(), obj.end() ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
