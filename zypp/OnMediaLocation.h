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
    * Describes a path ona certain media amongs as the information
    * required to download it, like its media number, checksum and
    * size.
    * it does not specifies the URI of the file.
  */
  class OnMediaLocation
  {
    friend std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  public:
    /** Ctor */
    OnMediaLocation()
    : _medianr( 1 )
    {}

  public:
    unsigned          medianr()        const { return _medianr; }
    const Pathname &  filename()       const { return _filename; }
    const CheckSum &  checksum()       const { return _checksum; }
    const ByteCount & downloadSize()   const { return _downloadsize; }
    const ByteCount & openSize() const { return _opendownloadsize; }
    const CheckSum &  openChecksum()     const { return _openchecksum; }

  public:
    OnMediaLocation & setMedianr( unsigned val_r )                 { _medianr = val_r; return *this; }
    OnMediaLocation & setFilename( const Pathname & val_r )        { _filename = val_r; return *this; }
    OnMediaLocation & setChecksum( const CheckSum & val_r )        { _checksum = val_r; return *this; }
    OnMediaLocation & setDownloadSize( const ByteCount & val_r )   { _downloadsize = val_r; return *this; }
    OnMediaLocation & setOpenChecksum( const CheckSum & val_r )      { _openchecksum = val_r; return *this; }
    OnMediaLocation & setOpenSize( const ByteCount & val_r ) { _opendownloadsize = val_r; return *this; }
  private:
    unsigned  _medianr;
    Pathname  _filename;
    CheckSum  _checksum;
    ByteCount _downloadsize;
    ByteCount _opendownloadsize;
    CheckSum _openchecksum;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates OnMediaLocation Stream output */
  std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ONMEDIALOCATION_H
