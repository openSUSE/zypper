/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ProductInfo.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/ProductInfo.h"
#include "zypp/PoolItem.h"
#include "zypp/Package.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace
  {
    /** Fast check whether a \ref sat::Solvable provides product metadata.
     * Return the solvable or \ref sat::noSolvable.
     */
    sat::Solvable ifProductInfoProvided( sat::Solvable solvable_r )
    {
#warning THIS SHOULD BE REPLACED BY A FAST TEST E.G. productName
      // and use detail::IdType lookupIdAttribute for this test

      if ( solvable_r.lookupStrAttribute( sat::SolvAttr::productShortlabel ).empty() )
        return sat::Solvable::noSolvable;
      return solvable_r;
    }

    /** List attribute helper */
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

    /** List attribute helper */
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

  const ProductInfo ProductInfo::noProductInfo;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ProductInfo::ProductInfo
  //	METHOD TYPE : Ctor
  //
  ProductInfo::ProductInfo()
  {}

  ProductInfo::ProductInfo( sat::Solvable solvable_r )
  : sat::Solvable( ifProductInfoProvided( solvable_r ) )
  {}

  ProductInfo::ProductInfo( const PoolItem & poolItem_r )
  : sat::Solvable( ifProductInfoProvided( poolItem_r.satSolvable() ) )
  {}

  ProductInfo::ProductInfo( const ResTraits<ResObject>::constPtrType & package_r )
  : sat::Solvable( package_r ? ifProductInfoProvided( package_r->satSolvable() )
                             : sat::Solvable::noSolvable )
  {}

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ProductInfo & obj )
  {
#warning provide reasonable output
    return str << obj.satSolvable();
  }

  ///////////////////////////////////////////////////////////////////

  PoolItem ProductInfo::poolItem() const
  {
    return PoolItem( *this );
  }

  ResTraits<Package>::constPtrType ProductInfo::package() const
  {
    return make<Package>( *this );
  }

  ///////////////////////////////////////////////////////////////////

#warning DUMMIES RETURNING THE PACKAGE ATTRIBUTES
  std::string ProductInfo::name() const
  { return sat::Solvable::name(); }

  Edition ProductInfo::edition() const
  { return sat::Solvable::edition(); }

  Arch ProductInfo::arch() const
  { return sat::Solvable::arch(); }

  Vendor ProductInfo::vendor() const
  { return sat::Solvable::vendor().asString(); }

  std::string ProductInfo::summary( const Locale & lang_r ) const
  { return package() ? package()->summary( lang_r ) : std::string(); }

  std::string ProductInfo::description( const Locale & lang_r ) const
  { return package() ? package()->description( lang_r ) : std::string(); }


#warning REVIEW THESE OLD PRODUCT ARTTRIBUTES

  std::string ProductInfo::type() const
  { return lookupStrAttribute( sat::SolvAttr::productType ); }

  Url ProductInfo::releaseNotesUrl() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productRelnotesurl );
    if ( ! ret.empty() )
      return  ret.front();
    return Url();
  }

  std::list<Url> ProductInfo::updateUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productUpdateurls );
    return ret;
  }

  std::list<Url> ProductInfo::extraUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productExtraurls );
    return ret;
  }

  std::list<Url> ProductInfo::optionalUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productOptionalurls );
    return ret;
  }

  std::list<std::string> ProductInfo::flags() const
  {
    std::list<std::string> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productFlags );
    return ret;
  }

  std::string ProductInfo::shortName() const
  { return lookupStrAttribute( sat::SolvAttr::productShortlabel ); }

  std::string ProductInfo::longName( const Locale & /*lang_r*/ ) const
  { return std::string(); }

  std::string ProductInfo::distributionName() const
  { return lookupStrAttribute( sat::SolvAttr::productDistproduct ); }

  Edition ProductInfo::distributionEdition() const
  { return Edition( lookupStrAttribute( sat::SolvAttr::productDistversion ) ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
