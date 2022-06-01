/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/ProvideSpec.h
 */
#ifndef ZYPP_MEDIA_PROVIDESPEC_H_INCLUDED
#define ZYPP_MEDIA_PROVIDESPEC_H_INCLUDED

#include <iosfwd>

#include <zypp-core/Url.h>
#include <zypp-core/ByteCount.h>
#include <zypp-core/CheckSum.h>
#include <zypp-core/TriBool.h>
#include <zypp-core/OnMediaLocation>
#include <zypp-media/ng/ProvideFwd>
#include <zypp-media/ng/HeaderValueMap>
#include <boost/iterator/iterator_adaptor.hpp>

namespace zyppng
{

  class ProvideMediaSpec
  {
  public:

    ProvideMediaSpec( const std::string &label, const zypp::Pathname &verifyData = zypp::Pathname(), unsigned medianr = 1 );

    /*!
     * The label of the medium, this will be shown in case a media change is required
     */
    const std::string &label() const;

    /*!
     * Changes the label of the medium
     */
    ProvideMediaSpec &setLabel( const std::string &label );

    /*!
     *  The media number the resource is located on.
     */
    unsigned medianr() const;

    /*!
     *  Individual manipulation of \c medianr (prefer \ref setLocation).
     */
    ProvideMediaSpec & setMedianr( unsigned medianr );

    /*!
     * Returns the media validation file path. This can be empty if there
     * is no file to validate.
     */
    zypp::Pathname mediaFile () const;

    /*!
     * Changes the media file to the name path \a pName.
     */
    ProvideMediaSpec &setMediaFile( const zypp::Pathname &pName );

    /*!
     * Returns a map of custom key->value pairs that can control special aspects
     * of how all files are provided by the medium after it was attached
     */
    HeaderValueMap &customHeaders();
    const HeaderValueMap &customHeaders() const;

    /*!
     * Set the custom header value identified by \a key to \a val
     */
    ProvideMediaSpec &setCustomHeaderValue (  const std::string &key, const HeaderValueMap::Value &val );

    /*!
     * Adds the custom header value \a val to the list of values identified by \a key
     */
    ProvideMediaSpec &addCustomHeaderValue (  const std::string &key, const HeaderValueMap::Value &val );

    zypp::TriBool isSameMedium ( const ProvideMediaSpec &other );

  public:
    class Impl;                 ///< Implementation class.
  private:
    zypp::RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
  };


  class ProvideFileSpec
  {
    friend std::ostream & operator<<( std::ostream &str, const ProvideFileSpec &obj );
    friend std::ostream & dumpOn( std::ostream &str, const ProvideFileSpec &obj );

  public:
    ProvideFileSpec();

    ProvideFileSpec( const zypp::OnMediaLocation &loc );

    /** Dtor */
    ~ProvideFileSpec();

    template <typename... T>
    static ProvideFileSpecRef create ( T... args ) {
      return std::make_shared<ProvideFileSpec>( std::forward<T>(args)... );
    }

    /*!
     * The destination file name, this is optional and is only
     * a hint to the \ref Provide instance where to put the file IF possible. The
     * file provider does not need to consider this.
     */
    const zypp::Pathname &destFilenameHint() const;
    ProvideFileSpec &setDestFilenameHint ( const zypp::Pathname &filename );

    bool checkExistsOnly () const;
    ProvideFileSpec & setCheckExistsOnly( const bool set = true );


    /** Whether this is an optional resource.
     * This is a hint to the downloader not to report an error if
     * the resource is not present on the server.
     */
    bool optional() const;
    /** Set whether the resource is \ref optional. */
    ProvideFileSpec & setOptional( bool val );

    /** The size of the resource on the server. */
    const zypp::ByteCount &downloadSize() const;
    /** Set the \ref downloadSize. */
    ProvideFileSpec &setDownloadSize( const zypp::ByteCount &val_r );

    /** The checksum of the resource on the server. */
    const zypp::CheckSum &checksum() const;
    /** Set the \ref checksum. */
    ProvideFileSpec &setChecksum( const zypp::CheckSum &val_r );


    /** The size of the resource once it has been uncompressed or unpacked. */
    const zypp::ByteCount &openSize() const;
    /** Set the \ref openSize. */
    ProvideFileSpec &setOpenSize( const zypp::ByteCount &val_r );

    /** The checksum of the resource once it has been uncompressed or unpacked. */
    const zypp::CheckSum &openChecksum() const;
    /** Set the \ref openChecksum. */
    ProvideFileSpec &setOpenChecksum( const zypp::CheckSum &val_r );

    /** The size of the header prepending the resource (e.g. for zchunk). */
    const zypp::ByteCount &headerSize() const;
    /** Set the \ref headerSize. */
    ProvideFileSpec &setHeaderSize( const zypp::ByteCount &val_r );

    /** The checksum of the header prepending the resource (e.g. for zchunk). */
    const zypp::CheckSum &headerChecksum() const;
    /** Set the \ref headerChecksum. */
    ProvideFileSpec &setHeaderChecksum( const zypp::CheckSum &val_r );

    /** The existing deltafile that can be used to reduce download size ( zchunk or metalink ) */
    const zypp::Pathname &deltafile() const;
    /** Set the \ref deltafile. */
    ProvideFileSpec &setDeltafile( const zypp::Pathname &path );

    /*!
     * Returns a map of custom key->value pairs that can control special aspects
     * of how the provide operation is processed.
     *
     * @todo should this actually just be a list of pair(string,string) instead of map? -> easier way to send multiple values to a worker
     */
    HeaderValueMap &customHeaders();
    const HeaderValueMap &customHeaders() const;

    /*!
     * Set the custom header value identified by \a key to \a val
     */
    ProvideFileSpec &setCustomHeaderValue (  const std::string &key, const HeaderValueMap::Value &val );

    /*!
     * Adds the custom header value \a val to the list of values identified by \a key
     */
    ProvideFileSpec &addCustomHeaderValue (  const std::string &key, const HeaderValueMap::Value &val );

  public:
    class Impl;                 ///< Implementation class.
  private:
    zypp::RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
  };

  /** \relates ProvideSpec Stream output */
  std::ostream & operator<<( std::ostream &str, const ProvideFileSpec &obj );

  /** \relates ProvideSpec Verbose stream output */
  std::ostream & dumpOn( std::ostream &str, const ProvideFileSpec &obj );

} // namespace zypp

#endif // ZYPP_MEDIA_PROVIDESPEC_H_INCLUDED
