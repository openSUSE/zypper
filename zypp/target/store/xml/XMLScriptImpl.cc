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
    {}
    /** Dtor */
    XMLScriptImpl::~XMLScriptImpl()
    {}

    Pathname XMLScriptImpl::do_script() const {
	return Pathname();
#warning FIXME
//      return _do_script;
    }

    Pathname XMLScriptImpl::undo_script() const {
	return Pathname();
#warning FIXME
//      return _undo_script;
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
