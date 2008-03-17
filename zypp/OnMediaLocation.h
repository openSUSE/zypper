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

#include "zypp/base/Deprecated.h"
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
    {}

    /** Ctor taking a filename and media number (defaults to 1). */
    OnMediaLocation( const Pathname & filename_r, unsigned medianr_r = 1 )
    : _medianr( medianr_r )
    , _filename( filename_r )
    {}

  public:
    unsigned          medianr()        const { return _medianr; }
    const Pathname &  filename()       const { return _filename; }
    const CheckSum &  checksum()       const { return _checksum; }
    const ByteCount & downloadSize()   const { return _downloadsize; }
    const ByteCount & openSize()       const { return _opendownloadsize; }
    const CheckSum &  openChecksum()   const { return _openchecksum; }

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

  public:
    /** Individual manipulation of \c medianr.
     * Using \ref setLocation is prefered.
    */
    OnMediaLocation & changeMedianr( unsigned val_r )
    { _medianr = val_r; return *this; }

    /** Individual manipulation of \c filename.
     * Using \ref setLocation is prefered.
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
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates OnMediaLocation Stream output */
  std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ONMEDIALOCATION_H
