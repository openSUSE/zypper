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

  std::string Product::type() const
  { return std::string(); }

  Label Product::vendor() const
  { return Label(); }

  Url Product::releaseNotesUrl() const
  { return Url(); }

  std::list<Url> Product::updateUrls() const
  { return std::list<Url>(); }

  std::list<Url> Product::extraUrls() const
  { return std::list<Url>(); }

  std::list<Url> Product::optionalUrls() const
  { return std::list<Url>(); }

  std::list<std::string> Product::flags() const
  { return std::list<std::string>(); }

  /** */
  Label Product::shortName() const
  { return Label(); }

  /** */
  Label Product::longName() const
  { return summary(); }

  std::string Product::distributionName() const
  { return std::string(); }

  Edition Product::distributionEdition() const
  { return Edition(); }


  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
