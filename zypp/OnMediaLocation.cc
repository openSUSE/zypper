/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/OnMediaLocation.cc
 *
*/
#include <iostream>
//#include <zypp/base/Logger.h>

#include <zypp/OnMediaLocation.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class OnMediaLocation::Impl
  /// \brief OnMediaLocation implementation.
  ///////////////////////////////////////////////////////////////////
  struct OnMediaLocation::Impl
  {
  public:
    Impl()
    {}

    Impl( Pathname filename_r, unsigned medianr_r )
    : _filename { std::move(filename_r) }
    , _medianr { medianr_r }
    {}

    Pathname _filename;
    unsigned _medianr = 0U;

    bool _optional = false;

    ByteCount _downloadSize;
    CheckSum  _checksum;

    ByteCount _openSize;
    CheckSum  _openChecksum;

    ByteCount _headerSize;
    CheckSum  _headerChecksum;

    Pathname  _deltafile;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    { static shared_ptr<Impl> _nullimpl( new Impl ); return _nullimpl; }
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const { return new Impl( *this ); }
  };

  /** \relates OnMediaLocation::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const OnMediaLocation::Impl & obj )
  { return str << "[" << obj._medianr << "]" << obj._filename << "{" << obj._downloadSize << "|" << obj._checksum << "|" << obj._deltafile <<  "}"; }

  /** \relates OnMediaLocation::Impl Verbose stream output */
  inline std::ostream & dumpOn( std::ostream & str, const OnMediaLocation::Impl & obj )
  { return str << obj; }

  ///////////////////////////////////////////////////////////////////
  //	CLASS NAME : OnMediaLocation
  ///////////////////////////////////////////////////////////////////

  OnMediaLocation::OnMediaLocation()
  : _pimpl( Impl::nullimpl() )
  {}

  OnMediaLocation::OnMediaLocation( Pathname filename_r, unsigned medianr_r )
  :  _pimpl( new Impl( std::move(filename_r), medianr_r ) )
  {}

  OnMediaLocation::~OnMediaLocation()
  {}


  const Pathname & OnMediaLocation::filename() const
  { return _pimpl->_filename; }

  unsigned OnMediaLocation::medianr() const
  { return _pimpl->_medianr; }

  OnMediaLocation & OnMediaLocation::setLocation( Pathname filename_r, unsigned medianr_r )
  { _pimpl->_filename = std::move(filename_r); _pimpl->_medianr = medianr_r; return *this; }

  OnMediaLocation & OnMediaLocation::unsetLocation()
  { _pimpl->_filename = Pathname(); _pimpl->_medianr = 0; return *this; }

  OnMediaLocation & OnMediaLocation::changeFilename( Pathname filename_r )
  { _pimpl->_filename = std::move(filename_r); return *this; }

  OnMediaLocation & OnMediaLocation::changeMedianr( unsigned medianr_r )
  { _pimpl->_medianr = medianr_r; return *this; }

  OnMediaLocation & OnMediaLocation::prependPath( const Pathname & prefix_r )
  { if ( ! prefix_r.emptyOrRoot() ) changeFilename( prefix_r / filename() ); return *this; }

  bool OnMediaLocation::optional() const
  { return _pimpl->_optional; }

  OnMediaLocation & OnMediaLocation::setOptional( bool val_r )
  { _pimpl->_optional = val_r; return *this; }

  const ByteCount & OnMediaLocation::downloadSize() const
  { return _pimpl->_downloadSize; }

  OnMediaLocation & OnMediaLocation::setDownloadSize( ByteCount val_r )
  { _pimpl->_downloadSize = val_r; return *this; }

  const CheckSum & OnMediaLocation::checksum() const
  { return _pimpl->_checksum; }

  OnMediaLocation & OnMediaLocation::setChecksum( CheckSum val_r )
  { _pimpl->_checksum = val_r; return *this; }

  const ByteCount & OnMediaLocation::openSize() const
  { return _pimpl->_openSize; }

  OnMediaLocation & OnMediaLocation::setOpenSize( ByteCount val_r )
  { _pimpl->_openSize = val_r; return *this; }

  const CheckSum & OnMediaLocation::openChecksum() const
  { return _pimpl->_openChecksum; }

  OnMediaLocation & OnMediaLocation::setOpenChecksum( CheckSum val_r )
  { _pimpl->_openChecksum = val_r; return *this; }

  const ByteCount & OnMediaLocation::headerSize() const
  { return _pimpl->_headerSize; }

  OnMediaLocation & OnMediaLocation::setHeaderSize( ByteCount val_r )
  { _pimpl->_headerSize = val_r; return *this; }

  const CheckSum & OnMediaLocation::headerChecksum() const
  { return _pimpl->_headerChecksum; }

  OnMediaLocation & OnMediaLocation::setHeaderChecksum( CheckSum val_r )
  { _pimpl->_headerChecksum = val_r; return *this; }

  const Pathname &OnMediaLocation::deltafile() const
  { return _pimpl->_deltafile; }

  OnMediaLocation &OnMediaLocation::setDeltafile( Pathname path )
  { _pimpl->_deltafile = std::move(path); return *this; }

  std::ostream & operator<<( std::ostream & str, const OnMediaLocation & obj )
  { return str << *obj._pimpl; }

  std::ostream & dumpOn( std::ostream & str, const OnMediaLocation & obj )
  { return dumpOn( str, *obj._pimpl ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
