/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zmd/backend/RpmCallbacks.cc
 *
*/

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
#include <zypp/target/rpm/RpmCallbacks.h>

///////////////////////////////////////////////////////////////////
namespace ZyppRecipients {
///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // InstallPkgCallback
    ///////////////////////////////////////////////////////////////////
    struct InstallPkgReceive : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
    {
	zypp::Resolvable::constPtr _last;
	int last_reported;

	InstallPkgReceive( )
	{
	}

	virtual void reportbegin()
	{
	}

	virtual void reportend()
	{
	}

	virtual void start(zypp::Resolvable::constPtr resolvable)
	{
	  // initialize the counter
	  last_reported = 0;

#warning install non-package
	  zypp::Package::constPtr res = zypp::asKind<zypp::Package>(resolvable);

	  // if we have started this resolvable already, don't do it again
	  if( _last == resolvable )
	    return;

	  std::cout << "1|" << "1|" << "Installing " << *resolvable;
	  _last = resolvable;
	}

	virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
	{
	    std::cout << "2|" << value << "|100";
	    return true;
	}

	virtual Action problem(
	  zypp::Resolvable::constPtr resolvable
	  , zypp::target::rpm::InstallResolvableReport::Error error
	  , std::string description
	  , zypp::target::rpm::InstallResolvableReport::RpmLevel level
	)
	{
	    if (level != zypp::target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE)
	    {
		DBG << "Retrying installation problem with too low severity (" << level << ")" << std::endl;
		return zypp::target::rpm::InstallResolvableReport::ABORT;
	    }

	    _last = zypp::Resolvable::constPtr();

#if 0
		// "R" =  retry
		if (ret == "R") return zypp::target::rpm::InstallResolvableReport::RETRY;

		// "C" = cancel
		if (ret == "C") return zypp::target::rpm::InstallResolvableReport::ABORT;

		// otherwise ignore
		return zypp::target::rpm::InstallResolvableReport::IGNORE;
#endif

	    return zypp::target::rpm::InstallResolvableReport::problem( resolvable, error, description, level );
	}

	virtual void finish(zypp::Resolvable::constPtr resolvable, Error error, std::string reason, zypp::target::rpm::InstallResolvableReport::RpmLevel level)
	{
	    if (level == zypp::target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE) {
		std::cout << "3|" << reason << std::endl;
	    }
	    else {
		std::cout << "4" << std::endl;
	    }
	}
    };


    ///////////////////////////////////////////////////////////////////
    // RemovePkgCallback
    ///////////////////////////////////////////////////////////////////
    struct RemovePkgReceive : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
    {
	RemovePkgReceive( )
	{
	}

	virtual void reportbegin()
	{
	}

	virtual void reportend()
	{
	}

	virtual void start(zypp::Resolvable::constPtr resolvable)
	{
	}

	virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
	{
	    std::cout << "2|" << value << "|100";
	    return true;
	}

	virtual void finish(zypp::Resolvable::constPtr resolvable, zypp::target::rpm::RemoveResolvableReport::Error error, std::string reason)
	{
	    if (error != NO_ERROR) {
		std::cout << "3|" << reason << std::endl;
	    }
	    else {
		std::cout << "4" << std::endl;
	    }
	}
    };


///////////////////////////////////////////////////////////////////
}; // namespace ZyppRecipients
///////////////////////////////////////////////////////////////////
