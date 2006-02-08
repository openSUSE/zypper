/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetImpl.cc
 *
*/
#include <iostream>
#include <string>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/target/TargetImpl.h"
#include "zypp/target/TargetCallbackReceiver.h"

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/InstallOrder.h"

using namespace std;
using zypp::solver::detail::InstallOrder;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(TargetImpl);

    TargetImpl_Ptr TargetImpl::_nullimpl;

    /** Null implementation */
    TargetImpl_Ptr TargetImpl::nullimpl()
    {
      if (_nullimpl == 0)
	_nullimpl = new TargetImpl;
      return _nullimpl;
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::TargetImpl
    //	METHOD TYPE : Ctor
    //
    TargetImpl::TargetImpl(const Pathname & root_r)
    : _root(root_r)
    {
      _rpm.initDatabase(_root);
      MIL << "Initialized target on " << _root << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::~TargetImpl
    //	METHOD TYPE : Dtor
    //
    TargetImpl::~TargetImpl()
    {
      _rpm.closeDatabase();
      MIL << "Targets closed" << endl;
    }

    const ResStore & TargetImpl::resolvables()
    {
      _store.clear();
      // RPM objects
      std::list<Package::Ptr> packages = _rpm.getPackages();
      for (std::list<Package::Ptr>::const_iterator it = packages.begin();
           it != packages.end();
           it++)
      {
        _store.insert(*it);
      }
      // TODO objects from the XML store
      return _store;
    }
    
    Pathname TargetImpl::getRpmFile(Package::constPtr package)
    {
	callback::SendReport<source::DownloadResolvableReport> report;

	// FIXME: error handling
	// FIXME: Url	
	report->start( package, Url() );

	Pathname file = package->getPlainRpm();

	report->finish( package, source::DownloadResolvableReport::NO_ERROR, "" );
	
	return file;
    }

    void TargetImpl::commit(ResPool pool_r)
    {
      MIL << "TargetImpl::commit(<pool>)" << endl;

      PoolItemList to_uninstall;
      PoolItemList to_install;
      PoolItemList installed;
      for (ResPool::const_iterator it = pool_r.begin();
           it != pool_r.end(); it++)
      {
	if (it->status().isToBeInstalled())
	{
	  to_install.push_back(*it);
	}
	else if (it->status().isToBeUninstalled()
		 && !it->status().isToBeUninstalledDueToObsolete())	// leave out obsoletes
	{
	  to_uninstall.push_back(*it);
	}
	else if (it->status().staysInstalled())
	{
	  installed.push_back(*it);
	}
      }

      if (to_uninstall.size() > 0 ) {

	// sort delete list...

	InstallOrder order(pool_r, to_uninstall, installed);
	order.init();
	const PoolItemList & uninstallorder(order.getTopSorted());

	to_uninstall.clear();
	for (PoolItemList::const_reverse_iterator it = uninstallorder.rbegin(); it != uninstallorder.rend(); ++it ) {
	    to_uninstall.push_back (*it);
	}

	// first uninstall what is to be uninstalled
	commit(to_uninstall);
      }

      // now install what is to be installed
      InstallOrder order(pool_r, to_install, installed);
      order.init();
      const PoolItemList & installorder(order.getTopSorted());
      commit(installorder);
    }

    void TargetImpl::commit(const PoolItemList & items_r)
    {
      MIL << "TargetImpl::commit(<list>)" << endl;
      for (PoolItemList::const_iterator it = items_r.begin();
	it != items_r.end(); it++)
      {
	if (isKind<Package>(it->resolvable()))
	{
	  Package::constPtr p = dynamic_pointer_cast<const Package>(it->resolvable());
	  if (it->status().isToBeInstalled())
	  {
	    Pathname localfile = getRpmFile(p);
	    
#warning Exception handling
	    // create a installation progress report proxy
    	    RpmInstallPackageReceiver progress(it->resolvable());
	    
	    progress.connect();

	    try {
		progress.tryLevel( target::rpm::InstallResolvableReport::RPM );
		
		rpm().installPackage(localfile,
		    p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : 0);
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT(excpt_r);

		WAR << "Install failed, retrying with --nodeps" << endl;
		try {
		    progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS );
		    rpm().installPackage(localfile,
			p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : rpm::RpmDb::RPMINST_NODEPS);
		}
		catch (Exception & excpt_r) {
		    ZYPP_CAUGHT(excpt_r);
		    WAR << "Install failed again, retrying with --force --nodeps" << endl;

		    try {
			progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE );
			rpm().installPackage(localfile,
			    p->installOnly() ? rpm::RpmDb::RPMINST_NOUPGRADE : (rpm::RpmDb::RPMINST_NODEPS|rpm::RpmDb::RPMINST_FORCE));
		    }
		    catch (Exception & excpt_r) {
			progress.disconnect();
			ZYPP_RETHROW(excpt_r);
		    }
		}
	    }
	      
	    progress.disconnect();

	  }
	  else
	  {
    	    RpmRemovePackageReceiver progress(it->resolvable());
	    
	    progress.connect();

	    try {
		rpm().removePackage(p);
	    }
	    catch (Exception & excpt_r) {
		ZYPP_CAUGHT(excpt_r);
		WAR << "Remove failed, retrying with --nodeps" << endl;
		rpm().removePackage(p, rpm::RpmDb::RPMINST_NODEPS);
	    }
	    progress.disconnect();
	  }

	  it->status().setStatus( ResStatus::uninstalled )
	  MIL << "Successful remove, " << *it << " is now uninstalled " << endl;
	}
#warning FIXME other resolvables
      }

    }

    rpm::RpmDb & TargetImpl::rpm()
    { return _rpm; }

    bool TargetImpl::providesFile (const std::string & path_str, const std::string & name_str) const
    { return _rpm.hasFile(path_str, name_str); }

      /** Return the resolvable which provides path_str (rpm -qf)
	  return NULL if no resolvable provides this file  */
    ResObject::constPtr TargetImpl::whoOwnsFile (const std::string & path_str) const
    {
	string name = _rpm.whoOwnsFile (path_str);
	if (name.empty())
	    return NULL;

	for (ResStore::const_iterator it = _store.begin(); it != _store.end(); ++it) {
	    if ((*it)->name() == name) {
		return *it;
	    }
	}
	return NULL;
    }



void
TargetImpl::getResolvablesToInsDel ( ResPool pool_r,
				    PoolItemList & dellist_r,
				    PoolItemList & instlist_r,
				    PoolItemList & srclist_r )
{
}


    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
