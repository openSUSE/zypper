/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetCallbackReceiver.h
 *
*/
#ifndef ZYPP_TARGET_TARGETCALLBACKRECEIVER_H
#define ZYPP_TARGET_TARGETCALLBACKRECEIVER_H

#include "zypp/ZYppCallbacks.h"
#include "zypp/target/rpm/RpmCallbacks.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    class RpmInstallPackageReceiver
	: public callback::ReceiveReport<rpm::RpmInstallReport>
    {
	callback::SendReport <rpm::InstallResolvableReport> _report;
	Resolvable::constPtr _resolvable;
	target::rpm::InstallResolvableReport::RpmLevel _level;
	bool _abort;
        std::string _finishInfo;

      public:

	RpmInstallPackageReceiver (Resolvable::constPtr res);
	virtual ~RpmInstallPackageReceiver ();

	virtual void reportbegin();

	virtual void reportend();

        /** Start the operation */
        virtual void start( const Pathname & name );

	void tryLevel( target::rpm::InstallResolvableReport::RpmLevel level_r );

	bool aborted() const { return _abort; }

        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent );

	/** inform user about a problem */
	virtual rpm::RpmInstallReport::Action problem( Exception & excpt_r );

        /** Additional rpm output to be reported in \ref finish in case of success. */
        virtual void finishInfo( const std::string & info_r );

        /** Finish operation in case of success */
        virtual void finish();

        /** Finish operatin in case of fail, report fail exception */
        virtual void finish( Exception & excpt_r );
    };

    class RpmRemovePackageReceiver
	: public callback::ReceiveReport<rpm::RpmRemoveReport>
    {
	callback::SendReport <rpm::RemoveResolvableReport> _report;
	Resolvable::constPtr _resolvable;
	bool _abort;
        std::string _finishInfo;

      public:

	RpmRemovePackageReceiver (Resolvable::constPtr res);
	virtual ~RpmRemovePackageReceiver ();

	virtual void reportbegin();

	virtual void reportend();

        /** Start the operation */
        virtual void start( const std::string & name );

        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent );

        /**
         *  Returns true if removing is aborted during progress
         */
	bool aborted() const { return _abort; }

	/** inform user about a problem */
	virtual rpm::RpmRemoveReport::Action problem( Exception & excpt_r );

        /** Additional rpm output to be reported in \ref finish in case of success. */
        virtual void finishInfo( const std::string & info_r );

        /** Finish operation in case of success */
        virtual void finish();

        /** Finish operatin in case of fail, report fail exception */
        virtual void finish( Exception & excpt_r );
    };

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_TARGETCALLBACKRECEIVER_H
