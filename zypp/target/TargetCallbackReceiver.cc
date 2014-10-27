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
#include <iostream>

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
	    , _level( target::rpm::InstallResolvableReport::RPM )
	    , _abort (false)
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
	    _abort = false;
	}

        /**
         * Inform about progress
         * Return true on abort
         */
        bool RpmInstallPackageReceiver::progress( unsigned percent )
	{
	    _abort = ! _report->progress( percent, _resolvable );
	    return _abort;
	}

	rpm::RpmInstallReport::Action
	RpmInstallPackageReceiver::problem( Exception & excpt_r )
	{
	    rpm::InstallResolvableReport::Action user =
		_report->problem( _resolvable
		    , rpm::InstallResolvableReport::INVALID
		    , excpt_r.asUserHistory()
		    , _level
		);

	    switch (user) {
		case rpm::InstallResolvableReport::RETRY:
		    return rpm::RpmInstallReport::RETRY;
		case rpm::InstallResolvableReport::ABORT:
                    _abort = true;
		    return rpm::RpmInstallReport::ABORT;
		case rpm::InstallResolvableReport::IGNORE:
		    return rpm::RpmInstallReport::IGNORE;
	    }

	    return rpm::RpmInstallReport::problem( excpt_r );
	}

        void RpmInstallPackageReceiver::finishInfo( const std::string & info_r )
        {
          _finishInfo = info_r;
        }

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::finish()
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::NO_ERROR, _finishInfo, _level );
	}

        /** Finish operation in case of success */
        void RpmInstallPackageReceiver::finish( Exception & excpt_r )
	{
	    _report->finish( _resolvable, rpm::InstallResolvableReport::INVALID, std::string(), _level );
	}

	void RpmInstallPackageReceiver::tryLevel( target::rpm::InstallResolvableReport::RpmLevel level_r )
	{
	    _level = level_r;
	}


	/////////////////////////////////////////////////////////////////
	///  RpmRemovePackageReceiver
	/////////////////////////////////////////////////////////////////

	RpmRemovePackageReceiver::RpmRemovePackageReceiver (Resolvable::constPtr res)
	    : callback::ReceiveReport<rpm::RpmRemoveReport> ()
	    , _resolvable (res)
            , _abort(false)
	{
	}

	RpmRemovePackageReceiver::~RpmRemovePackageReceiver ()
	{
	}

	void RpmRemovePackageReceiver::reportbegin()
	{
	}

	void RpmRemovePackageReceiver::reportend()
	{
	}

        /** Start the operation */
        void RpmRemovePackageReceiver::start( const std::string & name )
	{
	    _report->start( _resolvable );
            _abort = false;
	}

        /**
         * Inform about progress
         * Return true on abort
         */
        bool RpmRemovePackageReceiver::progress( unsigned percent )
	{
	    _abort = ! _report->progress( percent, _resolvable );
	    return _abort;
	}

	rpm::RpmRemoveReport::Action
	RpmRemovePackageReceiver::problem( Exception & excpt_r )
	{
	    rpm::RemoveResolvableReport::Action user =
		_report->problem( _resolvable
		    , rpm::RemoveResolvableReport::INVALID
		    , excpt_r.asUserHistory()
		);

	    switch (user) {
		case rpm::RemoveResolvableReport::RETRY:
		    return rpm::RpmRemoveReport::RETRY;
		case rpm::RemoveResolvableReport::ABORT:
                    _abort = true;
		    return rpm::RpmRemoveReport::ABORT;
		case rpm::RemoveResolvableReport::IGNORE:
		    return rpm::RpmRemoveReport::IGNORE;
	    }

	    return rpm::RpmRemoveReport::problem( excpt_r );
	}

        void RpmRemovePackageReceiver::finishInfo( const std::string & info_r )
        {
          _finishInfo = info_r;
        }

        /** Finish operation in case of success */
        void RpmRemovePackageReceiver::finish()
	{
            _report->progress( 100, _resolvable );
	    _report->finish( _resolvable, rpm::RemoveResolvableReport::NO_ERROR, _finishInfo );
	}

        /** Finish operation in case of success */
        void RpmRemovePackageReceiver::finish( Exception & excpt_r )
	{
	    _report->finish( _resolvable, rpm::RemoveResolvableReport::INVALID, std::string() );
	}

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
