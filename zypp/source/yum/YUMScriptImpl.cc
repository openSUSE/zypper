/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMScriptImpl.cc
*/

#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include <zypp/detail/ResObjectFactory.h>


using namespace std;
using namespace zypp::detail;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMScriptImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
       * \bug CANT BE CONSTUCTED THAT WAY ANYMORE
      */
      YUMScriptImpl::YUMScriptImpl(
	const zypp::parser::yum::YUMPatchScript & parsed
      )
      {
	_do_script = parsed.do_script;
	_undo_script = parsed.undo_script;
      }
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
