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

      public:

	RpmInstallPackageReceiver (Resolvable::constPtr res);
	virtual ~RpmInstallPackageReceiver ();
	
	virtual void reportbegin();
	
	virtual void reportend();

        /** Start the operation */
        virtual void start( const Pathname & name );

        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( unsigned percent );

        /** Finish operation in case of success */
        virtual void end();

        /** Finish operatin in case of fail, report fail exception */
        virtual void end( Exception & excpt_r );
    };
	
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_TARGEtCALLBACKRECEIVER_H
