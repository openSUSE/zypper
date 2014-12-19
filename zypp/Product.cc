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
#include "zypp/base/StrMatcher.h"

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
    // Look for a  provider of 'product(name) = version' of same
    // architecture and within the same repo.
    //
    // bnc #497696: Update repos may have multiple release package versions
    // providing the same product. As a workaround we link to the one with
    // the highest version.
    Capability identCap( str::form( "product(%s) = %s", name().c_str(), edition().c_str() ) );

    sat::Solvable found;
    sat::WhatProvides providers( identCap );
    for_( it, providers.begin(), providers.end() )
    {
      if ( it->repository() == repository()
           && it->arch() == arch() )
      {
        if ( ! found || found.edition() < it->edition() )
          found = *it;
      }
    }

    if ( ! found && isSystem() )
    {
      // bnc#784900: for installed products check whether the file is owned by
      // some package. If so, ust this as buddy.
      sat::LookupAttr q( sat::SolvAttr::filelist, repository() );
      std::string refFile( referenceFilename() );
      if ( ! refFile.empty() )
      {
	StrMatcher matcher( referenceFilename() );
	q.setStrMatcher( matcher );
	if ( ! q.empty() )
	  found = q.begin().inSolvable();
      }
      else
	INT << "Product referenceFilename unexpectedly empty!" << endl;
    }

    if ( ! found )
      WAR << *this << ": no reference package found: " << identCap << endl;
    return found;
  }

  std::string Product::referenceFilename() const
  { return lookupStrAttribute( sat::SolvAttr::productReferenceFile ); }

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

  CapabilitySet Product::droplist() const
  { return poolItem().buddy().valuesOfNamespace( "weakremover" ); }

  std::string Product::productLine() const
  { return lookupStrAttribute( sat::SolvAttr::productProductLine ); }

  ///////////////////////////////////////////////////////////////////

  std::string Product::shortName() const
  {
    std::string ret( lookupStrAttribute( sat::SolvAttr::productShortlabel ) );
    if ( ret.empty() ) ret = name();
    return ret;

  }

  std::string Product::flavor() const
  {
    // Look for a  provider of 'product_flavor(name) = version'
    // within the same repo. Unlike the reference package, we
    // can be relaxed and ignore the architecture.
    Capability identCap( str::form( "product_flavor(%s) = %s", name().c_str(), edition().c_str() ) );

    sat::WhatProvides providers( identCap );
    for_( it, providers.begin(), providers.end() )
    {
      if ( it->repository() == repository() )
      {
        // Got the package now try to get the provided 'flavor(...)'
        Capabilities provides( it->provides() );
        for_( cap, provides.begin(), provides.end() )
        {
          std::string capstr( cap->asString() );
          if ( str::hasPrefix( capstr, "flavor(" ) )
          {
            capstr = str::stripPrefix( capstr, "flavor(" );
            capstr.erase( capstr.size()-1 ); // trailing ')'
            return capstr;
          }
        }
      }
    }
    return std::string();
  }

  std::string Product::type() const
  { return lookupStrAttribute( sat::SolvAttr::productType ); }

  std::list<std::string> Product::flags() const
  {
    std::list<std::string> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productFlags );
    return ret;
  }

  Date Product::endOfLife() const
  { return Date( lookupNumAttribute( sat::SolvAttr::productEndOfLife ) );}

  std::vector<Repository::ContentIdentifier> Product::updateContentIdentifier() const
  {
    std::vector<Repository::ContentIdentifier> ret;
    sat::LookupAttr q( sat::SolvAttr::productUpdatesRepoid, sat::SolvAttr::productUpdates, *this );
    if ( ! q.empty() )
    {
      ret.reserve( 2 );
      for_( it, q.begin(), q.end() )
	ret.push_back( it.asString() );
    }
    return ret;
  }

  bool Product::hasUpdateContentIdentifier( const Repository::ContentIdentifier & cident_r ) const
  {
    sat::LookupAttr q( sat::SolvAttr::productUpdatesRepoid, sat::SolvAttr::productUpdates, *this );
    for_( it, q.begin(), q.end() )
    {
      if ( it.asString() == cident_r )
	return true;
    }
    return false;
  }

  bool Product::isTargetDistribution() const
  { return isSystem() && lookupStrAttribute( sat::SolvAttr::productType ) == "base"; }

  std::string Product::registerTarget() const
  { return lookupStrAttribute( sat::SolvAttr::productRegisterTarget ); }

  std::string Product::registerRelease() const
  { return lookupStrAttribute( sat::SolvAttr::productRegisterRelease ); }

  std::string Product::registerFlavor() const
  { return lookupStrAttribute( sat::SolvAttr::productRegisterFlavor ); }
  
  /////////////////////////////////////////////////////////////////

  Product::UrlList Product::urls( const std::string & key_r ) const
  {
    UrlList ret;

    sat::LookupAttr url( sat::SolvAttr::productUrl, *this );
    sat::LookupAttr url_type( sat::SolvAttr::productUrlType, *this );

    sat::LookupAttr::iterator url_it(url.begin());
    sat::LookupAttr::iterator url_type_it(url_type.begin());

    for (;url_it != url.end(); ++url_it, ++url_type_it)
    {
        /* safety checks, shouldn't happen (tm) */
        if (url_type_it == url_type.end())
        {
            ERR << *this << " : The thing that should not happen, happened." << endl;
            break;
        }

        if ( url_type_it.asString() == key_r )
        {
            ret._list.push_back(url_it.asString());
        }
    } /* while (attribute array) */

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
