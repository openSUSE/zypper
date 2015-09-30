/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/MediaProducts.h
 * Functions to find out products in media
 */
#ifndef ZYPP_MEDIAPRODUCTS_H_
#define ZYPP_MEDIAPRODUCTS_H_

#include <iterator>
#include <iostream>
#include <fstream>
#include "zypp/ZConfig.h"
#include "zypp/base/Logger.h"
#include "zypp/media/MediaManager.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/ProgressData.h"

namespace zypp
{
  /**
   * \short Represents an available product in media
   */
  struct MediaProductEntry
  {
    Pathname    _dir;
    std::string _name;

    /**
     * \short Ctor
     */
    MediaProductEntry( const Pathname & dir_r = "/", const std::string & name_r = std::string() )
      : _dir(dir_r), _name(name_r)
    {
    }

    bool operator<( const MediaProductEntry &rhs ) const
    {
      return ( _name < rhs._name );
    }
  };

  /**
   * A set of available products in media
   */
  typedef std::set<MediaProductEntry> MediaProductSet;

  /**
   * FIXME: add a comment here...
   */
  template <class TOutputIterator>
  static void scanProductsFile( const Pathname & file_r, TOutputIterator result )
  {
    std::ifstream pfile( file_r.asString().c_str() );
    while ( pfile.good() ) {

      std::string value = str::getline( pfile, str::TRIM );
      if ( pfile.bad() ) {
        ERR << "Error parsing " << file_r << std::endl;
        ZYPP_THROW(Exception("Error parsing " + file_r.asString()));
      }
      if ( pfile.fail() ) {
        break; // no data on last line
      }
      std::string tag = str::stripFirstWord( value, true );

      if ( tag.size() ) {
        *result = MediaProductEntry( tag, value );
      }
    }
  }

  /**
   * \short Available products in a url location
   *
   * \param url_r url to inspect
   * \param result output iterator where \ref MediaProductEntry
   * items will be inserted.
   * \throws MediaException If accessng the media fails
   */
  template <class TOutputIterator>
  void productsInMedia( const Url & url_r, TOutputIterator result )
  {
    media::MediaManager media_mgr;
    // open the media
    media::MediaId id = media_mgr.open(url_r);
    media_mgr.attach(id);
    Pathname products_file = Pathname("media.1/products");

    try  {
      media_mgr.provideFile (id, products_file);
      products_file = media_mgr.localPath (id, products_file);
      scanProductsFile (products_file, result);
    }
    catch ( const Exception & excpt ) {
      ZYPP_CAUGHT(excpt);
      MIL << "No products description found on the Url" << std::endl;
    }
    media_mgr.release(id, "");
 }

 /**
   * \short Available products in a url location
   *
   * \param url_r url to inspect
   * \param set ef MediaProductEntry set where
   * items will be inserted.
   * \throws MediaException If accessng the media fails
   */
  void productsInMedia( const Url & url_r, MediaProductSet &set )
  {
    productsInMedia(url_r, std::inserter(set, set.end()));
  }

} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
