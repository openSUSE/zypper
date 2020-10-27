/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/OnMediaLocation.h
 */
#ifndef ZYPP_SOURCE_ONMEDIALOCATION_H
#define ZYPP_SOURCE_ONMEDIALOCATION_H

#include <iosfwd>

#include <zypp/base/PtrTypes.h>

#include <zypp/APIConfig.h>
#include <zypp/Pathname.h>
#include <zypp/ByteCount.h>
#include <zypp/CheckSum.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class OnMediaLocation
  /// \brief Describes a resource file located on a medium.
  ///
  /// Holds the path of a resource on a medium and contains additional
  /// info required to retrieve and verify it (like media number,
  /// checksum, size,...)
  ///
  /// It does not specify the \ref Url of the medium itself.
  ///
  /// Media number \c 0 usually indicates no media access.
  ///////////////////////////////////////////////////////////////////
  class OnMediaLocation
  {
    friend std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );
    friend std::ostream & dumpOn( std::ostream & str, const OnMediaLocation & obj );

  public:
    /** Default Ctor indicating no media access. */
    OnMediaLocation();

    /** Ctor taking a \a filename_r and \a medianr_r (defaults to \c 1). */
    OnMediaLocation( Pathname filename_r, unsigned medianr_r = 1 );

    /** Dtor */
    ~OnMediaLocation();

  public:
    /** The path to the resource on the medium. */
    const Pathname & filename() const;

    /** The media number the resource is located on. */
    unsigned medianr() const;


    /** Set \a filename_r and \a medianr_r (defaults to \c 1). */
    OnMediaLocation & setLocation( Pathname filename_r, unsigned medianr_r = 1 );

    /** Unset \c filename and set \c medianr to \c 0. */
    OnMediaLocation & unsetLocation();


    /** Individual manipulation of \c filename (prefer \ref setLocation). */
    OnMediaLocation & changeFilename( Pathname filename_r );

    /** Individual manipulation of \c medianr (prefer \ref setLocation). */
    OnMediaLocation & changeMedianr( unsigned medianr_r );

    /** Prepend the filename with \a prefix_r */
    OnMediaLocation & prependPath( const Pathname & prefix_r );

  public:
    /** Whether this is an optional resource.
     * This is a hint to the downloader not to report an error if
     * the resource is not present on the server.
     */
    bool optional() const;
    /** Set whether the resource is \ref optional. */
    OnMediaLocation & setOptional( bool val );

  public:
    /** The size of the resource on the server. */
    const ByteCount & downloadSize() const;
    /** Set the \ref downloadSize. */
    OnMediaLocation & setDownloadSize( ByteCount val_r );

    /** The checksum of the resource on the server. */
    const CheckSum & checksum() const;
    /** Set the \ref checksum. */
    OnMediaLocation & setChecksum( CheckSum val_r );

  public:
    /** The size of the resource once it has been uncompressed or unpacked. */
    const ByteCount & openSize() const;
    /** Set the \ref openSize. */
    OnMediaLocation & setOpenSize( ByteCount val_r );

    /** The checksum of the resource once it has been uncompressed or unpacked. */
    const CheckSum & openChecksum() const;
    /** Set the \ref openChecksum. */
    OnMediaLocation & setOpenChecksum( CheckSum val_r );

  public:
    /** The size of the header prepending the resource (e.g. for zchunk). */
    const ByteCount & headerSize() const;
    /** Set the \ref headerSize. */
    OnMediaLocation & setHeaderSize( ByteCount val_r );

    /** The checksum of the header prepending the resource (e.g. for zchunk). */
    const CheckSum & headerChecksum() const;
    /** Set the \ref headerChecksum. */
    OnMediaLocation & setHeaderChecksum( CheckSum val_r );

    /** The existing deltafile that can be used to reduce download size ( zchunk or metalink ) */
    const Pathname & deltafile() const;
    /** Set the \ref deltafile. */
    OnMediaLocation & setDeltafile( Pathname path );


  public:
    class Impl;                 ///< Implementation class.
  private:
    RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
  };

  /** \relates OnMediaLocation Stream output */
  std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj );

  /** \relates OnMediaLocation Verbose stream output */
  std::ostream & dumOn( std::ostream & str, const OnMediaLocation & obj );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ONMEDIALOCATION_H
