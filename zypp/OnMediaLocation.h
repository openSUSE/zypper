/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/OnMediaLocation.h
 *
*/
#ifndef ZYPP_SOURCE_ONMEDIALOCATION_H
#define ZYPP_SOURCE_ONMEDIALOCATION_H

#include <iosfwd>

#include "zypp/APIConfig.h"
#include "zypp/Pathname.h"
#include "zypp/ByteCount.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : OnMediaLocation
  //
  /**
   * Describes a path on a certain media amongs as the information
   * required to download it, like its media number, checksum and
   * size. It does not specify the URI of the file.
   *
   * Media number \c 0 usually indicates no media access.
   *
   * \todo Implement cheap copy via COW.
  */
  class OnMediaLocation
  {
    friend std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  public:
    /** Default ctor indicating no media access. */
    OnMediaLocation()
    : _medianr( 0 )
    , _optional(false)
    {}

    /** Ctor taking a filename and media number (defaults to 1). */
    OnMediaLocation( const Pathname & filename_r, unsigned medianr_r = 1 )
    : _medianr( medianr_r )
    , _filename( filename_r )
    , _optional(false) // bnc #447010
    {}

  public:
    /**
     * media number where the resource is located.
     * for a url cd:// this could be 1..N.
     * for a url of type http://host/path/CD1, a media number 2
     * means looking on http://host/path/CD1/../CD2
     */
    unsigned          medianr()        const { return _medianr; }
    /**
     * The path to the resource relatve to the url and path.
     * If the base is http://novell.com/download/repository, the
     * resource filename could be "/repodata/repomd.xml"
     */
    const Pathname &  filename()       const { return _filename; }
    /**
     * the checksum of the resource
     */
    const CheckSum &  checksum()       const { return _checksum; }
    /**
     * The size of the resource on the server. Therefore
     * the size of the download.
     */
    const ByteCount & downloadSize()   const { return _downloadsize; }
    /**
     * The size of the resource once it has been uncompressed
     * or unpacked.
     * If the file is file.txt.gz then this is the size of
     * file.txt
     */
    const ByteCount & openSize()       const { return _opendownloadsize; }
    /**
     * The checksum of the resource once it has been uncompressed
     * or unpacked.
     * If the file is file.txt.gz then this is the checksum of
     * file.txt
     */
    const CheckSum &  openChecksum()   const { return _openchecksum; }
    /**
     * whether this is an optional resource. That is a file that
     * may not be present. This is just a hint to the resource
     * downloader to not error in case the not found resource is
     * not found.
     */
    const bool optional() const { return _optional; }

  public:
    /** Unset \c filename and set \c medianr to \c 0. */
    OnMediaLocation & unsetLocation()
    { _filename = Pathname(); _medianr = 0; return *this; }

    /** Set filename and media number (defaults to \c 1). */
    OnMediaLocation & setLocation( const Pathname & val_r,
                                   unsigned mediaNumber_r = 1 )
    { _filename = val_r; _medianr = mediaNumber_r; return *this; }

   /** Set the files size. */
    OnMediaLocation & setDownloadSize( const ByteCount & val_r )
    { _downloadsize = val_r; return *this; }

    /** Set the files checksum. */
    OnMediaLocation & setChecksum( const CheckSum & val_r )
    { _checksum = val_r; return *this; }

    /** Set the files open (uncompressed) size. */
    OnMediaLocation & setOpenSize( const ByteCount & val_r )
    { _opendownloadsize = val_r; return *this; }

    /** Set the files open (uncompressed) checksum. */
    OnMediaLocation & setOpenChecksum( const CheckSum & val_r )
    { _openchecksum = val_r; return *this; }

    /**
     * Set the whether the resource is optional or not
     * \see optional
     */
    OnMediaLocation & setOptional( bool val )
    { _optional = val; return *this; }

  public:
   /**
    * Individual manipulation of \c medianr (prefer \ref setLocation).
    * Using \ref setLocation is prefered as us usually have to adjust
    * \c filename and \c medianr in sync.
    */
    OnMediaLocation & changeMedianr( unsigned val_r )
    { _medianr = val_r; return *this; }

    /**
     * Individual manipulation of \c filename (prefer \ref setLocation).
     * Using \ref setLocation is preferedas us usually have to adjust
     * \c filename and \c medianr in sync.
     */
    OnMediaLocation & changeFilename( const Pathname & val_r )
    { _filename = val_r; return *this; }

  private:
    unsigned  _medianr;
    Pathname  _filename;
    CheckSum  _checksum;
    ByteCount _downloadsize;
    ByteCount _opendownloadsize;
    CheckSum  _openchecksum;
    bool      _optional;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates OnMediaLocation Stream output */
  std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ONMEDIALOCATION_H
