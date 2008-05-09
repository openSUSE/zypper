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
  //
  //	Package interface forwarded to implementation
  //
  ///////////////////////////////////////////////////////////////////

  std::string Product::type() const
  { return lookupStrAttribute( sat::SolvAttr::productType ); }

  Url Product::releaseNotesUrl() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productRelnotesurl );
    if ( ! ret.empty() )
      return  ret.front();
    return Url();
  }

  std::list<Url> Product::updateUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productUpdateurls );
    return ret;
  }

  std::list<Url> Product::extraUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productExtraurls );
    return ret;
  }

  std::list<Url> Product::optionalUrls() const
  {
    std::list<Url> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productOptionalurls );
    return ret;
  }

  std::list<std::string> Product::flags() const
  {
    std::list<std::string> ret;
    fillList( ret, satSolvable(), sat::SolvAttr::productFlags );
    return ret;
  }

  std::string Product::shortName() const
  { return lookupStrAttribute( sat::SolvAttr::productShortlabel ); }

  std::string Product::longName( const Locale & lang_r ) const
  { return summary( lang_r ); }

  std::string Product::distributionName() const
  { return lookupStrAttribute( sat::SolvAttr::productDistproduct ); }

  Edition Product::distributionEdition() const
  { return Edition( lookupStrAttribute( sat::SolvAttr::productDistversion ) ); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
