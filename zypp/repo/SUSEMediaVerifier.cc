/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include <zypp/base/Logger.h>
#include <zypp/base/Gettext.h>
#include <zypp/repo/SUSEMediaVerifier.h>
#include <zypp/media/MediaHandler.h>

using std::endl;

namespace zypp
{
  namespace repo
  {
    ///////////////////////////////////////////////////////////////////
    /// Data parsed from a media.1/media file
    ///
    struct SMVData
    {
      SMVData( const Pathname & path_r )
      {
        std::ifstream inp( path_r.c_str() );
        if ( !inp ) {
          ERR << "Can't setup a SUSEMediaVerifier from file: " << path_r.asString() << endl;
          return;
        }
        getline( inp, _mediaVendor );
        getline( inp, _mediaIdent );
        std::string buffer;
        getline( inp, buffer );
        str::strtonum( buffer, _totalMedia );
        //if ( !_totalMedia ) _totalMedia = 1;
      }

      /** Validate object in a boolean context: valid */
      explicit operator bool() const
      { return valid(); }

      /** Data considered to be valid if we have vendor and ident. */
      bool valid() const
      { return ! (_mediaVendor.empty() || _mediaIdent.empty()); }

      /** Whether \a rhs belongs to the same media set. */
      bool matches( const SMVData & rhs ) const
      { return valid() && rhs._mediaVendor == _mediaVendor && rhs._mediaIdent == _mediaIdent; }

      std::string    _mediaVendor;
      std::string    _mediaIdent;
      media::MediaNr _totalMedia = 0;
    };

    /** \relates SMVData Stream output  */
    inline std::ostream & operator<<( std::ostream & str, const SMVData & obj )
    { return str << "[" << obj._mediaVendor << "|" << obj._mediaIdent << "/" << obj._totalMedia << "]"; }


    ///////////////////////////////////////////////////////////////////
    /// SUSEMediaVerifier::Impl
    ///
    /// Data parsed from the media.1/media file can be shared among different
    /// verifiers for this media.
    ///
    /// This type of verifier however is convenience to tell in advance if the
    /// wrong media is inserted. Errors when retrieving the data are logged,
    /// but won't let any verification fail. If the medium happens to be right,
    /// everything is fine. If it's wrong, download time will tell.
    ///
    class SUSEMediaVerifier::Impl
    {
    public:
      Impl( const Pathname & path_r, media::MediaNr mediaNr_r )
      : _smvData { new SMVData( path_r ) }
      , _mediaNr { mediaNr_r }
      {}

      Impl( const Impl & rhs, media::MediaNr mediaNr_r )
      : _smvData { rhs._smvData }
      , _mediaNr { mediaNr_r }
      {}

      const SMVData & smvData() const
      { return *_smvData; }

      media::MediaNr mediaNr() const
      { return _mediaNr; }

      Pathname mediaFilePath( media::MediaNr mediaNr_r = 0 ) const
      {
        str::Format fmt { "/media.%d/media" };
        fmt % str::numstring( mediaNr_r ? mediaNr_r : _mediaNr );
        return fmt.str();
      }

      std::string expectedAsUserString() const
      {
        // Translator: %1% the expected medium number; %2% the total number of media in the set; %3% the ident file on the medium.
        str::Format fmt { _("Expected medium %1%/%2% identified by file '%3%' with content:") };
        return str::Str()
        << ( fmt % mediaNr() % smvData()._totalMedia % mediaFilePath() ) << endl
        << "    " << smvData()._mediaVendor  << endl
        << "    " << smvData()._mediaIdent;
      }

    private:
      shared_ptr<SMVData> _smvData;
      media::MediaNr _mediaNr = 1;
    };

    /** \relates SUSEMediaVerifier::Impl Stream output  */
    inline std::ostream & operator<<( std::ostream & str, const SUSEMediaVerifier::Impl & obj )
    { return str << obj.smvData() << "(" << obj.mediaNr() << ")"; }


    ///////////////////////////////////////////////////////////////////
    // SUSEMediaVerifier
    ///////////////////////////////////////////////////////////////////

    SUSEMediaVerifier::SUSEMediaVerifier( const Pathname & path_r, media::MediaNr mediaNr_r )
    : _pimpl { new Impl( path_r, mediaNr_r ) }
    {}

    SUSEMediaVerifier::SUSEMediaVerifier( const SUSEMediaVerifier & rhs, media::MediaNr mediaNr_r )
    : _pimpl { new Impl( *rhs._pimpl, mediaNr_r ) }
    {}

    SUSEMediaVerifier::~SUSEMediaVerifier()
    {}

    bool SUSEMediaVerifier::valid() const
    { return _pimpl->smvData().valid(); }

    const std::string & SUSEMediaVerifier::vendor() const
    { return _pimpl->smvData()._mediaVendor; }

    const std::string & SUSEMediaVerifier::ident() const
    { return _pimpl->smvData()._mediaIdent; }

    media::MediaNr SUSEMediaVerifier::totalMedia() const
    { return _pimpl->smvData()._totalMedia; }

    media::MediaNr SUSEMediaVerifier::mediaNr() const
    { return _pimpl->mediaNr(); }


    bool SUSEMediaVerifier::isDesiredMedia( const media::MediaHandler & ref ) const
    {
      bool ret = true;	// optimistic return unless we definitely know it does not match

      const SMVData & smvData = _pimpl->smvData();
      if ( ! smvData )
        return ret;	// we have no valid data

      // bsc#1180851: If there is just one not-volatile medium in the set
      // tolerate a missing (vanished) media identifier and let the URL rule.
      bool relaxed = smvData._totalMedia == 1 && ! Url::schemeIsVolatile( ref.protocol() );
      SEC << smvData << endl;
      SEC << ref.protocol() << " " <<  Url::schemeIsVolatile( ref.protocol() ) << endl;

      Pathname mediaFile { _pimpl->mediaFilePath() };
      try {
        ref.provideFile( OnMediaLocation(mediaFile) );
        mediaFile = ref.localPath( mediaFile );
      }
      catch ( media::MediaFileNotFoundException & excpt_r )
      {
        if ( relaxed ) {
          ZYPP_CAUGHT( excpt_r );
          return ret;
        }
        excpt_r.addHistory( _pimpl->expectedAsUserString() );
        ZYPP_RETHROW( excpt_r );
      }
      catch ( media::MediaNotAFileException & excpt_r )
      {
        if ( relaxed ) {
          ZYPP_CAUGHT( excpt_r );
          return ret;
        }
        excpt_r.addHistory( _pimpl->expectedAsUserString() );
        ZYPP_CAUGHT( excpt_r ); return ret;
      }

      SMVData remote { mediaFile };
      ret = smvData.matches( remote );
      if ( ! ret ) {
        DBG << "expect: " << smvData << " medium " << mediaNr() << endl;
        DBG << "remote: " << remote  << endl;
      }
      return ret;
    }

    std::ostream & operator<<( std::ostream & str, const SUSEMediaVerifier & obj )
    { return str << *obj._pimpl; }

  } // namespace repo
} // namespace zypp

