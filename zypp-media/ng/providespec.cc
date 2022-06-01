/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp-media/providespec.cc
 *
*/

#include <iostream>
#include "providespec.h"

using std::endl;

namespace zyppng
{
  class ProvideSpecBasePrivate
  {
  public:
    ProvideSpecBasePrivate() {}
    virtual ~ProvideSpecBasePrivate() {}
    HeaderValueMap _customHeaders;
  };


  class ProvideMediaSpec::Impl : public ProvideSpecBasePrivate
  {
  public:
    Impl()
    {}

    Impl( const std::string &label, const zypp::Pathname &vPath, unsigned medianr )
      : _label( label )
      , _medianr( medianr )
      , _verifyDataPath(vPath)
    {}

    std::string _label;
    unsigned _medianr = 0U;
    zypp::Pathname _verifyDataPath;

  public:
    /** Offer default Impl. */
    static zypp::shared_ptr<Impl> nullimpl()
    { static zypp::shared_ptr<Impl> _nullimpl( new Impl ); return _nullimpl; }

  private:
    friend ProvideMediaSpec::Impl * zypp::rwcowClone<ProvideMediaSpec::Impl>( const ProvideMediaSpec::Impl * rhs );
    Impl * clone() const { return new Impl( *this ); }
  };

  class ProvideFileSpec::Impl : public ProvideSpecBasePrivate
  {
  public:
    Impl()
    {}

    zypp::Pathname _destFilenameHint;
    zypp::Pathname _mediaSpecFile;
    bool _checkExistsOnly = false;

    bool _optional = false;
    zypp::ByteCount _downloadSize;
    zypp::CheckSum  _checksum;

    zypp::ByteCount _openSize;
    zypp::CheckSum  _openChecksum;

    zypp::ByteCount _headerSize;
    zypp::CheckSum  _headerChecksum;

    zypp::Pathname  _deltafile;


  public:
    /** Offer default Impl. */
    static zypp::shared_ptr<Impl> nullimpl()
    { static zypp::shared_ptr<Impl> _nullimpl( new Impl ); return _nullimpl; }

  private:
    friend ProvideFileSpec::Impl * zypp::rwcowClone<ProvideFileSpec::Impl>( const ProvideFileSpec::Impl * rhs );
    Impl * clone() const { return new Impl( *this ); }
  };


  ProvideMediaSpec::ProvideMediaSpec( const std::string &label, const zypp::filesystem::Pathname &verifyData, unsigned medianr )
    :  _pimpl( new Impl( label, verifyData, medianr ) )
  {

  }

  const std::string &ProvideMediaSpec::label() const
  { return _pimpl->_label; }

  ProvideMediaSpec &ProvideMediaSpec::setLabel(const std::string &label)
  {
    _pimpl->_label = label;
    return *this;
  }

  unsigned ProvideMediaSpec::medianr() const
  { return _pimpl->_medianr; }

  ProvideMediaSpec &ProvideMediaSpec::setMedianr(unsigned medianr)
  {
    _pimpl->_medianr = medianr;
    return *this;
  }

  zypp::filesystem::Pathname ProvideMediaSpec::mediaFile() const
  { return _pimpl->_verifyDataPath; }

  ProvideMediaSpec &ProvideMediaSpec::setMediaFile(const zypp::filesystem::Pathname &pName)
  {
    _pimpl->_verifyDataPath = pName;
    return *this;
  }

  HeaderValueMap &ProvideMediaSpec::customHeaders()
  { return _pimpl->_customHeaders; }

  const HeaderValueMap &ProvideMediaSpec::customHeaders() const
  { return _pimpl->_customHeaders; }

  ProvideMediaSpec &ProvideMediaSpec::setCustomHeaderValue(const std::string &key, const HeaderValueMap::Value &val)
  {
    _pimpl->_customHeaders.set( key,val );
    return *this;
  }

  ProvideMediaSpec &ProvideMediaSpec::addCustomHeaderValue(const std::string &key, const HeaderValueMap::Value &val)
  {
    _pimpl->_customHeaders.add( key,val );
    return *this;
  }

  zypp::TriBool ProvideMediaSpec::isSameMedium( const ProvideMediaSpec &other )
  {
    // first check if we have the same media data
    if ( _pimpl->_verifyDataPath != other._pimpl->_verifyDataPath )
      return false;

    // if the verify file is not empty check the medianr
    if ( !_pimpl->_verifyDataPath.empty() ) {
      return _pimpl->_medianr == other._pimpl->_medianr;
    }

    // can't tell without the URL
    return zypp::indeterminate;
  }

  /** \relates ProvideSpec::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const ProvideFileSpec::Impl & obj )
  {
    return str << "{" << obj._destFilenameHint << "{" << obj._downloadSize << "|" << obj._checksum << "|" << obj._deltafile <<  "}" <<  "}";
  }

  /** \relates ProvideSpec::Impl Verbose stream output */
  inline std::ostream & dumpOn( std::ostream & str, const ProvideFileSpec::Impl & obj )
  { return str << obj; }


  ProvideFileSpec::ProvideFileSpec()
  : _pimpl( new Impl() )
  {}

  ProvideFileSpec::ProvideFileSpec( const zypp::OnMediaLocation &loc )
  : _pimpl( new Impl() )
  {
    setDownloadSize( loc.downloadSize() );
    setOptional( loc.optional() );
    setChecksum( loc.checksum() );
    setOpenSize( loc.openSize() );
    setOpenChecksum( loc.openChecksum() );
    setHeaderSize( loc.headerSize() );
    setHeaderChecksum( loc.headerChecksum() );
    setDeltafile( loc.deltafile() );
  }

  ProvideFileSpec::~ProvideFileSpec()
  {}

  const zypp::filesystem::Pathname &ProvideFileSpec::destFilenameHint() const
  { return _pimpl->_destFilenameHint; }

  ProvideFileSpec &ProvideFileSpec::setDestFilenameHint(const zypp::filesystem::Pathname &filename)
  { _pimpl->_destFilenameHint = filename; return *this; }

  bool ProvideFileSpec::checkExistsOnly() const
  { return _pimpl->_checkExistsOnly; }

  ProvideFileSpec &ProvideFileSpec::setCheckExistsOnly(const bool set)
  { _pimpl->_checkExistsOnly = set; return *this; }

  bool ProvideFileSpec::optional() const
  { return _pimpl->_optional; }

  ProvideFileSpec & ProvideFileSpec::setOptional( bool val_r )
  { _pimpl->_optional = (val_r); return *this; }

  const zypp::ByteCount & ProvideFileSpec::downloadSize() const
  { return _pimpl->_downloadSize; }

  ProvideFileSpec & ProvideFileSpec::setDownloadSize( const zypp::ByteCount &val_r )
  { _pimpl->_downloadSize = (val_r); return *this; }

  const zypp::CheckSum & ProvideFileSpec::checksum() const
  { return _pimpl->_checksum; }

  ProvideFileSpec & ProvideFileSpec::setChecksum( const zypp::CheckSum &val_r )
  { _pimpl->_checksum = (val_r); return *this; }

  const zypp::ByteCount & ProvideFileSpec::openSize() const
  { return _pimpl->_openSize; }

  ProvideFileSpec & ProvideFileSpec::setOpenSize( const zypp::ByteCount &val_r )
  { _pimpl->_openSize = (val_r); return *this; }

  const zypp::CheckSum & ProvideFileSpec::openChecksum() const
  { return _pimpl->_openChecksum; }

  ProvideFileSpec & ProvideFileSpec::setOpenChecksum( const zypp::CheckSum &val_r )
  { _pimpl->_openChecksum = (val_r); return *this; }

  const zypp::ByteCount & ProvideFileSpec::headerSize() const
  { return _pimpl->_headerSize; }

  ProvideFileSpec & ProvideFileSpec::setHeaderSize( const zypp::ByteCount &val_r )
  { _pimpl->_headerSize = (val_r); return *this; }

  const zypp::CheckSum & ProvideFileSpec::headerChecksum() const
  { return _pimpl->_headerChecksum; }

  ProvideFileSpec & ProvideFileSpec::setHeaderChecksum( const zypp::CheckSum &val_r )
  { _pimpl->_headerChecksum = (val_r); return *this; }

  const zypp::Pathname &ProvideFileSpec::deltafile() const
  { return _pimpl->_deltafile; }

  ProvideFileSpec &ProvideFileSpec::setDeltafile( const zypp::Pathname &path )
  { _pimpl->_deltafile = (path); return *this; }

  HeaderValueMap &ProvideFileSpec::customHeaders()
  { return _pimpl->_customHeaders; }

  const HeaderValueMap &ProvideFileSpec::customHeaders() const
  { return _pimpl->_customHeaders; }

  ProvideFileSpec &ProvideFileSpec::setCustomHeaderValue(const std::string &key, const HeaderValueMap::Value &val)
  {
    _pimpl->_customHeaders.set( key,val );
    return *this;
  }

  ProvideFileSpec &ProvideFileSpec::addCustomHeaderValue(const std::string &key, const HeaderValueMap::Value &val)
  {
    _pimpl->_customHeaders.add( key,val );
    return *this;
  }

  std::ostream & operator<<( std::ostream & str, const ProvideFileSpec & obj )
  { return str << *obj._pimpl; }

  std::ostream & dumpOn( std::ostream & str, const ProvideFileSpec & obj )
  { return dumpOn( str, *obj._pimpl ); }

} // namespace zypp
