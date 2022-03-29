/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_MEDIA_NG_MEDIAVERIFIER_H_INCLUDED
#define ZYPP_MEDIA_NG_MEDIAVERIFIER_H_INCLUDED

#include <string>
#include <zypp-core/zyppng/core/ByteArray>
#include <zypp-media/ng/ProvideFwd>

namespace zypp {
  namespace filesystem {
    class Pathname;
  }
  using filesystem::Pathname;
}

namespace zyppng {

  /*!
   * The MediaDataVerifier is used to verify if a specific medium, e.g. a DVD, is the
   * medium we actually need. Each verifier is defined by a verifier type and the data
   * that is used to validate against the medium.
   */
  class MediaDataVerifier
  {
  public:

    MediaDataVerifier() noexcept;
    virtual ~MediaDataVerifier();

    static MediaDataVerifierRef createVerifier ( const std::string &verifierType );

    /** Data considered to be valid if we have vendor and ident. */
    virtual bool valid() const = 0;

    /** Whether \a rhs belongs to the same media set. */
    virtual bool matches( const MediaDataVerifierRef & rhs ) const = 0;

    /*!
     * Returns the media vendor string
     */
    virtual const std::string &mediaVendor() const = 0;

    /*!
     * Returns the media ident string
     */
    virtual const std::string &mediaIdent() const = 0;

    /*!
     * Returns the total number of mediums in this set
     */
    virtual uint totalMedia() const = 0;

    /*!
     * Writes the mediaverifier data to stream
     */
    virtual std::ostream & toStream ( std::ostream & str ) const = 0;

    /*!
     * Load verification information from a given file, all media data must
     * be storeable in a file so that the controller can store a copy of it somewhere.
     */
    virtual bool load( const zypp::Pathname &data ) = 0;

    /*!
     * Generates the file information from a mounted medium, the path given in \a data
     * is the mountpoint of the device.
     */
    virtual bool loadFromMedium( const zypp::Pathname &data, uint mediaNr ) = 0;

    /*!
     * Returns the path of the media identifier file on the medium
     */
    virtual zypp::Pathname mediaFilePath ( uint mediaNr ) const = 0;

    /*!
     * Clones \a this and returns a reference to the clone
     */
    virtual MediaDataVerifierRef clone () const = 0;

    /*!
     * Returns a error string describing the expected medium.
     */
    virtual std::string expectedAsUserString( uint mediaNr = 1 ) const = 0;
  };

  /** \relates Stream output  */
  std::ostream & operator<<( std::ostream & str, const MediaDataVerifierRef & obj );



}

#endif
