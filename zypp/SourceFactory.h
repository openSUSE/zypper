/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceFactory.h
 *
*/
#ifndef ZYPP_SOURCEFACTORY_H
#define ZYPP_SOURCEFACTORY_H

#include <iosfwd>
#include <string>
#include <set>

#include "zypp/base/PtrTypes.h"

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceFactory
  //
  /** Factory to create a \ref Source_Ref.
   * Actually a Singleton
   *
  */
  class SourceFactory
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceFactory & obj );

  public:
    /** Default ctor */
    SourceFactory();
    /** Dtor */
    ~SourceFactory();

  public:
    /** Construct source from an implementation.
     * Returns Source_Ref::noSource on NULL \a impl_r.
    */
    Source_Ref createFrom( const Source_Ref::Impl_Ptr & impl_r );

    /** Construct source.
     * \throw EXCEPTION on fail
    */
    Source_Ref createFrom( const Url & url_r, const Pathname & path_r = "/", const std::string & alias_r = "", const Pathname & cache_dir_r = "", const bool base_source = false );

    /** Construct source of a given type.
     * \throw EXCEPTION on fail
    */
    Source_Ref createFrom( const std::string & type,  const Url & url_r, const Pathname & path_r = "/", const std::string & alias_r = "", const Pathname & cache_dir_r = "", const bool base_source = false );

  private:
    /** Implementation  */
    class Impl;
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;

  public:
   struct ProductEntry {
      Pathname    _dir;
      std::string _name;
      ProductEntry( const Pathname & dir_r = "/", const std::string & name_r = std::string() ){
        _dir  = dir_r;
        _name = name_r;
      }
      bool operator<( const ProductEntry & rhs ) const {
        return( _dir.asString() < rhs._dir.asString() );
      }
    };

    typedef std::set<ProductEntry> ProductSet;

    /** Check which products are available on the media
     * \throw Exception or MediaException on fail
     */
    void listProducts( const Url & url_r, ProductSet & products_r );

  private:
    void scanProductsFile( const Pathname & file_r, ProductSet & pset_r ) const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceFactory Stream output */
  extern std::ostream & operator<<( std::ostream & str, const SourceFactory & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEFACTORY_H
