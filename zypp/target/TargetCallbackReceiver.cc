/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetCallbackReceiver.cc
 *
*/

#include "zypp/target/TargetCallbackReceiver.h"

#include "zypp/target/rpm/RpmCallbacks.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

	RpmInstallPackageReceiver::RpmInstallPackageReceiver (Resolvable::constPtr res)
	    : callback::ReceiveReport<rpm::RpmInstallReport> ()
	    , _resolvable (res)
	{
	}

	RpmInstallPackageReceiver::~RpmInstallPackageReceiver ()
	{
	}
	
	void RpmInstallPackageReceiver::reportbegin() 
	{
	}
	
	void RpmInstallPackageReceiver::reportend() 
	{
	}

        /** Start the operation */
        void RpmInstallPackageReceiver::start( const Pathname & name ) 
	{
	    _report->start( _resolvable );
	}

        /**
         * Inform about progress
         * Return true on abort
         */
        bool RpmInstallPackageReceiver::progress( unsigned percent )
	{
	    return _report->progress( percent, _resolvable );
	}

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::end()
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::NO_ERROR, std::string() );
	}

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::end( Exception & excpt_r )
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::INVALID, std::string() );
	}

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
