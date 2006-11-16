/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLPatchImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/target/store/xml/XMLPatchImpl.h"
#include "zypp/Package.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLPatchImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    XMLPatchImpl::XMLPatchImpl()
    {}

    /** Dtor */
    XMLPatchImpl::~XMLPatchImpl()
    {}

    std::string XMLPatchImpl::id() const
    {
      return _patch_id;
    }
    Date XMLPatchImpl::timestamp() const
    {
      return _timestamp;
    }

    std::string XMLPatchImpl::category() const
    {
      return _category;
    }

    bool XMLPatchImpl::reboot_needed() const
    {
      return _reboot_needed;
    }

    bool XMLPatchImpl::affects_pkg_manager() const
    {
      return _affects_pkg_manager;
    }

    XMLPatchImpl::AtomList XMLPatchImpl::all_atoms() const {
      return _atoms;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
