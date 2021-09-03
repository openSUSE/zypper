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

#include <zypp-core/base/String.h>
#include <zypp-core/base/StringV.h>

#include <zypp/ZConfig.h>
#include <zypp/ZYppCommitPolicy.h>
#include <zypp-core/base/LogControl.h>
#include <zypp-core/TriBool.h>
#include <zypp/PathInfo.h>

#ifdef NO_SINGLETRANS_USERMERGE
#include <zypp/ZYppCallbacks.h>
#endif
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  inline bool ImZYPPER()
  { return filesystem::readlink( "/proc/self/exe" ).basename() == "zypper"; }

  bool singleTransInEnv ()
  {
#ifdef SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER
    static bool singleTrans = ImZYPPER();
#else // SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER
    static bool singleTrans = ImZYPPER() && ([]()->bool{
      const char *val = ::getenv("ZYPP_SINGLE_RPMTRANS");
#ifdef NO_SINGLETRANS_USERMERGE
      // Bug 1189788 - UsrMerge: filesystem package breaks system when upgraded in a single rpm transaction
      // While the bug is not fixed, we don't allow ZYPP_SINGLE_RPMTRANS=1 on a not UsrMerged system.
      // I.e. if /lib is a directory and not a symlink.
      bool ret = ( val && std::string_view( val ) == "1"  );
      if ( ret && PathInfo( "/lib", PathInfo::LSTAT ).isDir() ) {
	WAR << "Ignore $ZYPP_SINGLE_RPMTRANS=1: Bug 1189788 - UsrMerge: filesystem package breaks system when upgraded in a single rpm transaction" << std::endl;
	JobReport::info(
	"[boo#1189788] Tumbleweeds filesystem package seems to be unable to perform the\n"
	"              UsrMerge reliably in a single transaction. The requested\n"
	"              $ZYPP_SINGLE_RPMTRANS=1 will therefore be IGNORED because\n"
	"              the UsrMerge did not yet happen on this system."
	, JobReport::UserData( "cmdout", "[boo#1189788]" ) );
	return false;
      }
      return ret;
#else
      return ( val && std::string_view( val ) == "1"  );
#endif
#endif // SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER
    })();
    return singleTrans;
  }

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
      , _downloadMode		( ZConfig::instance().commit_downloadMode() )
      , _rpmInstFlags		( ZConfig::instance().rpmInstallFlags() )
      , _syncPoolAfterCommit	( true )
      , _singleTransMode        ( singleTransInEnv() )
      {}

    public:
      unsigned			_restrictToMedia;
      DownloadMode		_downloadMode;
      target::rpm::RpmInstFlags	_rpmInstFlags;
      bool			_syncPoolAfterCommit;
      bool                      _singleTransMode; //< run everything in one big rpm transaction

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
  {  _pimpl->_rpmInstFlags.setFlag( target::rpm::RPMINST_TEST, yesNo_r ); return *this; }

  bool ZYppCommitPolicy::dryRun() const
  { return _pimpl->_rpmInstFlags.testFlag( target::rpm::RPMINST_TEST );}

  ZYppCommitPolicy & ZYppCommitPolicy::downloadMode( DownloadMode val_r )
  {
    if ( singleTransModeEnabled() && val_r == DownloadAsNeeded ) {
      DBG << val_r << " is not compatible with singleTransMode, falling back to " << DownloadInAdvance << std::endl;
      _pimpl->_downloadMode = DownloadInAdvance;
    }
    _pimpl->_downloadMode = val_r; return *this;
  }

  DownloadMode ZYppCommitPolicy::downloadMode() const
  {
    if ( singleTransModeEnabled() && _pimpl->_downloadMode == DownloadAsNeeded ) {
      DBG << _pimpl->_downloadMode << " is not compatible with singleTransMode, falling back to " << DownloadInAdvance << std::endl;
      return DownloadInAdvance;
    }
    return _pimpl->_downloadMode;
  }

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

  ZYppCommitPolicy &ZYppCommitPolicy::allowDowngrade(bool yesNo_r)
  { _pimpl->_rpmInstFlags.setFlag( target::rpm::RPMINST_ALLOWDOWNGRADE, yesNo_r ); return *this; }

  bool ZYppCommitPolicy::allowDowngrade() const
  { return _pimpl->_rpmInstFlags.testFlag( target::rpm::RPMINST_ALLOWDOWNGRADE ); }

  ZYppCommitPolicy &ZYppCommitPolicy::replaceFiles( bool yesNo_r )
  { _pimpl->_rpmInstFlags.setFlag( target::rpm::RPMINST_REPLACEFILES, yesNo_r ); return *this; }

  bool ZYppCommitPolicy::replaceFiles( ) const
  { return _pimpl->_rpmInstFlags.testFlag( target::rpm::RPMINST_REPLACEFILES ); }

  ZYppCommitPolicy & ZYppCommitPolicy::syncPoolAfterCommit( bool yesNo_r )
  { _pimpl->_syncPoolAfterCommit = yesNo_r; return *this; }

  bool ZYppCommitPolicy::syncPoolAfterCommit() const
  { return _pimpl->_syncPoolAfterCommit; }

  bool ZYppCommitPolicy::singleTransModeEnabled() const
  { return _pimpl->_singleTransMode; }

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
