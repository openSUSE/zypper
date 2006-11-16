/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/PatchImpl.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/detail/PatchImpl.h"
#include "zypp/Package.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatchImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    PatchImpl::PatchImpl()
    {}

    /** Dtor */
    PatchImpl::~PatchImpl()
    {}

    std::string PatchImpl::id() const
    {
      return _patch_id;
    }
    Date PatchImpl::timestamp() const
    {
      return _timestamp;
    }

    TranslatedText PatchImpl::summary() const
    {
      return _summary;
    }

    TranslatedText PatchImpl::description() const
    {
      return _description;
    }

    std::string PatchImpl::category() const
    {
      return _category;
    }

    bool PatchImpl::reboot_needed() const
    {
      return _reboot_needed;
    }

    bool PatchImpl::affects_pkg_manager() const
    {
      return _affects_pkg_manager;
    }

    PatchImpl::AtomList PatchImpl::all_atoms() const {
      return _atoms;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
