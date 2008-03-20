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
#include "zypp/Product.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Product);

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
#warning DUMMY type
  std::string Product::type() const
  { return std::string(); }

#warning DUMMY releaseNotesUrl
  Url Product::releaseNotesUrl() const
  { return Url(); }

#warning DUMMY updateUrls
  std::list<Url> Product::updateUrls() const
  { return std::list<Url>(); }

#warning DUMMY extraUrls
  std::list<Url> Product::extraUrls() const
  { return std::list<Url>(); }

#warning DUMMY optionalUrls
  std::list<Url> Product::optionalUrls() const
  { return std::list<Url>(); }

#warning DUMMY flags
  std::list<std::string> Product::flags() const
  { return std::list<std::string>(); }

#warning DUMMY shortName
  std::string Product::shortName() const
  { return std::string(); }

  std::string Product::longName( const Locale & lang_r ) const
  { return summary( lang_r ); }

#warning DUMMY distributionName
  std::string Product::distributionName() const
  { return std::string(); }

#warning DUMMY distributionEdition
  Edition Product::distributionEdition() const
  { return Edition(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
