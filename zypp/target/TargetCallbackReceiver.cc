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
	
	rpm::RpmInstallReport::Action 
	RpmInstallPackageReceiver::problem( Exception & excpt_r )
	{
	    rpm::InstallResolvableReport::Action user = 
		_report->problem( _resolvable
		    , rpm::InstallResolvableReport::INVALID
		    , excpt_r.msg()
		);
		
	    switch (user) {
		case rpm::InstallResolvableReport::RETRY: 
		    return rpm::RpmInstallReport::RETRY;
		case rpm::InstallResolvableReport::ABORT: 
		    return rpm::RpmInstallReport::ABORT;
		case rpm::InstallResolvableReport::IGNORE: 
		    return rpm::RpmInstallReport::IGNORE;
	    }
	    
	    return rpm::RpmInstallReport::problem( excpt_r );
	}

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::finish()
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::NO_ERROR, std::string() );
	}

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::finish( Exception & excpt_r )
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::INVALID, std::string() );
	}

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
