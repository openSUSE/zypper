/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "mediaverifier.h"
#include <ostream>
#include <fstream>
#include <zypp-core/Pathname.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Gettext.h>
#include "private/providedbg_p.h"

namespace zyppng {

  ZYPP_FWD_DECL_TYPE_WITH_REFS (SuseMediaDataVerifier);

  class SuseMediaDataVerifier : public MediaDataVerifier
  {
  public:
    // MediaDataVerifier interface
    bool valid() const override;
    bool matches(const MediaDataVerifierRef &rhs) const override;
    const std::string &mediaVendor() const override;
    const std::string &mediaIdent() const override;
    uint totalMedia() const override;
    std::ostream &toStream(std::ostream &str) const override;
    bool load( const zypp::Pathname &data ) override;
    bool loadFromMedium(const zypp::filesystem::Pathname &data, uint expectedMediaNr ) override;
    zypp::filesystem::Pathname mediaFilePath(uint mediaNr) const override;
    MediaDataVerifierRef clone () const override;
    std::string expectedAsUserString( uint mediaNr ) const override;

  private:
    std::string _mediaVendor;
    std::string _mediaIdent;
    uint        _totalMedia = 0;
  };

  zypp::Pathname SuseMediaDataVerifier::mediaFilePath(uint mediaNr) const
  {
    zypp::str::Format fmt { "/media.%d/media" };
    return (fmt % zypp::str::numstring( mediaNr )).asString();
  }

  bool SuseMediaDataVerifier::loadFromMedium( const zypp::filesystem::Pathname &data, uint expectedMediaNr )
  {
    return load ( data / mediaFilePath(expectedMediaNr) );
  }

  bool SuseMediaDataVerifier::valid() const
  { return ! (_mediaVendor.empty() || _mediaIdent.empty()); }

  bool SuseMediaDataVerifier::matches(const MediaDataVerifierRef &rhs) const
  {
    auto conv = std::dynamic_pointer_cast<SuseMediaDataVerifier>(rhs);
    return conv && valid() && conv->_mediaVendor == _mediaVendor && conv->_mediaIdent == _mediaIdent;
  }

  uint SuseMediaDataVerifier::totalMedia() const
  {
    return _totalMedia;
  }

  std::ostream &SuseMediaDataVerifier::toStream( std::ostream &str ) const
  {
    return str << "[" << _mediaVendor << "|" << _mediaIdent << "/" << _totalMedia << "]";
  }

  bool SuseMediaDataVerifier::load( const zypp::Pathname &path_r )
  {
    std::ifstream inp( path_r.c_str() );
    if ( !inp ) {
      ERR << "Can't setup a SUSEMediaVerifier from file: " << path_r.asString() << std::endl;
      return false;
    }
    getline( inp, _mediaVendor );
    getline( inp, _mediaIdent );
    std::string buffer;
    getline( inp, buffer );
    zypp::str::strtonum( buffer, _totalMedia );
    //if ( !_totalMedia ) _totalMedia = 1;
    // loaded but maybe not valid
    return true;
  }

  const std::string &SuseMediaDataVerifier::mediaIdent() const
  {
    return _mediaIdent;
  }

  const std::string &SuseMediaDataVerifier::mediaVendor() const
  {
    return _mediaVendor;
  }

  MediaDataVerifierRef SuseMediaDataVerifier::clone () const
  {
    return SuseMediaDataVerifierRef( new SuseMediaDataVerifier( *this ) );
  }

  std::string SuseMediaDataVerifier::expectedAsUserString( uint mediaNr ) const
  {
    // Translator: %1% the expected medium number; %2% the total number of media in the set; %3% the ident file on the medium.
    zypp::str::Format fmt { _("Expected medium %1%/%2% identified by file '%3%' with content:") };
    return zypp::str::Str()
    << ( fmt % mediaNr % _totalMedia % mediaFilePath( mediaNr ) ) << "\n"
    << "    " << _mediaVendor  << "\n"
    << "    " << _mediaIdent;
  }

  MediaDataVerifier::MediaDataVerifier() noexcept
  { }

  MediaDataVerifier::~MediaDataVerifier()
  { }

  MediaDataVerifierRef MediaDataVerifier::createVerifier( const std::string &verifierType )
  {
    if ( verifierType == "SuseMediaV1" ) {
      return SuseMediaDataVerifierRef( new SuseMediaDataVerifier() );
    }
    return nullptr;
  }

  std::ostream &operator<<(std::ostream &str, const MediaDataVerifierRef &obj)
  {
    if ( obj )
      return obj->toStream(str);
    return str << "[MediaVerifier: null]";
  }

}
