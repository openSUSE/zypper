/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZYppCommitPolicy.cc
 *
*/

#include <iostream>

#include "zypp/base/String.h"

#include "zypp/ZConfig.h"
#include "zypp/ZYppCommitPolicy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitPolicy::Impl
  //
  ///////////////////////////////////////////////////////////////////

  class ZYppCommitPolicy::Impl
  {
    public:
      Impl()
      : _restrictToMedia	( 0 )
      , _dryRun			( false )
      , _downloadMode		( ZConfig::instance().commit_downloadMode() )
      , _rpmInstFlags		( ZConfig::instance().rpmInstallFlags() )
      , _syncPoolAfterCommit	( true )
      {}

    public:
      unsigned			_restrictToMedia;
      bool			_dryRun;
      DownloadMode		_downloadMode;
      target::rpm::RpmInstFlags	_rpmInstFlags;
      bool			_syncPoolAfterCommit;

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const { return new Impl( *this ); }
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZYppCommitPolicy
  //
  ///////////////////////////////////////////////////////////////////

  ZYppCommitPolicy::ZYppCommitPolicy()
  : _pimpl( new Impl )
  {}


  ZYppCommitPolicy & ZYppCommitPolicy::restrictToMedia( unsigned mediaNr_r )
  { _pimpl->_restrictToMedia = ( mediaNr_r == 1 ) ? 1 : 0; return *this; }

  unsigned ZYppCommitPolicy::restrictToMedia() const
  { return _pimpl->_restrictToMedia; }


  ZYppCommitPolicy & ZYppCommitPolicy::dryRun( bool yesNo_r )
  { _pimpl->_dryRun = yesNo_r; return *this; }

  bool ZYppCommitPolicy::dryRun() const
  { return _pimpl->_dryRun; }


  ZYppCommitPolicy & ZYppCommitPolicy::downloadMode( DownloadMode val_r )
  { _pimpl->_downloadMode = val_r; return *this; }

  DownloadMode ZYppCommitPolicy::downloadMode() const
  { return _pimpl->_downloadMode; }


  ZYppCommitPolicy &  ZYppCommitPolicy::rpmInstFlags( target::rpm::RpmInstFlags newFlags_r )
  { _pimpl->_rpmInstFlags = newFlags_r; return *this; }

  ZYppCommitPolicy & ZYppCommitPolicy::rpmNoSignature( bool yesNo_r )
  { _pimpl->_rpmInstFlags.setFlag( target::rpm::RPMINST_NOSIGNATURE, yesNo_r ); return *this; }

  ZYppCommitPolicy & ZYppCommitPolicy::rpmExcludeDocs( bool yesNo_r )
  { _pimpl->_rpmInstFlags.setFlag( target::rpm::RPMINST_EXCLUDEDOCS, yesNo_r ); return *this; }

  target::rpm::RpmInstFlags ZYppCommitPolicy::rpmInstFlags() const
  { return _pimpl->_rpmInstFlags; }

  bool ZYppCommitPolicy::rpmNoSignature() const
  { return _pimpl->_rpmInstFlags.testFlag( target::rpm::RPMINST_NOSIGNATURE ); }

  bool ZYppCommitPolicy::rpmExcludeDocs() const
  { return _pimpl->_rpmInstFlags.testFlag( target::rpm::RPMINST_EXCLUDEDOCS ); }


  ZYppCommitPolicy & ZYppCommitPolicy::syncPoolAfterCommit( bool yesNo_r )
  { _pimpl->_syncPoolAfterCommit = yesNo_r; return *this; }

  bool ZYppCommitPolicy::syncPoolAfterCommit() const
  { return _pimpl->_syncPoolAfterCommit; }


  std::ostream & operator<<( std::ostream & str, const ZYppCommitPolicy & obj )
  {
    str << "CommitPolicy(";
    if ( obj.restrictToMedia() )
      str << " restrictToMedia:" << obj.restrictToMedia();
    if ( obj.dryRun() )
      str << " dryRun";
    str << " " << obj.downloadMode();
    if ( obj.syncPoolAfterCommit() )
      str << " syncPoolAfterCommit";
    if ( obj.rpmInstFlags() )
      str << " rpmInstFlags{" << str::hexstring(obj.rpmInstFlags()) << "}";
    return str << " )";
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
