/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/XMLScriptImpl.cc
 *
*/

#include "zypp/target/store/xml/XMLScriptImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : XMLScriptImpl
    //
    ///////////////////////////////////////////////////////////////////

    /** Default ctor */
    XMLScriptImpl::XMLScriptImpl()
    {
      _do_script = TmpFile( TmpPath::defaultLocation(), "zypp-xmlstore-do-script-");
      _undo_script = TmpFile( TmpPath::defaultLocation(), "zypp-xmlstore-undo-script-");
    }
    
    /** Dtor */
    XMLScriptImpl::~XMLScriptImpl()
    {}

    Pathname XMLScriptImpl::do_script() const {
      return _do_script.path();
    }

    Pathname XMLScriptImpl::undo_script() const {
        return _undo_script.path();
    }

    bool XMLScriptImpl::undo_available() const {
      return _undo_script != "";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
