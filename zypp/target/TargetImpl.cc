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
#include "zypp/base/Logger.h"

#include "zypp/target/TargetImpl.h"

#include <vector>

using namespace std;

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
      _rpm.initDatabase();
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

    void TargetImpl::commit(ResPool pool_r)
    {
#warning FIXME this orderding doesn't honor the dependencies
      vector<ResObject::constPtr> to_uninstall;
      vector<ResObject::constPtr> to_install;
      for (ResPool::const_iterator it = pool_r.begin();
           it != pool_r.end(); it++)
      {
	if (it->status().isToBeInstalled())
	{
	  to_install.push_back(it->resolvable());
	}
	else if (it->status().isToBeUninstalled())
	{
	  to_uninstall.push_back(it->resolvable());
	}
      }
      // first uninstall what is to be uninstalled
      for (vector<ResObject::constPtr>::const_iterator it = to_uninstall.begin();
           it != to_uninstall.end();
	   it++)
      {
	if (isKind<Package>(*it))
	{
	  Package::constPtr p = dynamic_pointer_cast<const Package>(*it);
	  rpm().removePackage(p);
	}
#warning FIXME other resolvables (once more below)
      }
      // now install what is to be installed
      for (vector<ResObject::constPtr>::const_iterator it = to_install.begin();
           it != to_install.end();
	   it++)
      {
	if (isKind<Package>(*it))
	{
	  Package::constPtr p = dynamic_pointer_cast<const Package>(*it);
	  rpm().installPackage(p->getPlainRpm(), rpm::RpmDb::RPMINST_NOUPGRADE);
	}
      }
    }

    rpm::RpmDb & TargetImpl::rpm()
    { return _rpm; }

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
