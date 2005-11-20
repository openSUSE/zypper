/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUM
 ScriptImpl.cc
 *
*/

#include "zypp/source/yum/YUMScriptImpl.h"

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
#warning MA: CANT BE CONSTUCTED THAT WAY ANYMORE
#if 0
      : ScriptImpl(
	parsed.name,
	Edition(parsed.ver, parsed.rel, strtol(parsed.epoch.c_str(), NULL, 10)),
	Arch("noarch")

      )
#endif
      {
	_do_script = parsed.do_script;
	_undo_script = parsed.undo_script;
/*
    std::list<YUMDependency> provides;
    std::list<YUMDependency> conflicts;
    std::list<YUMDependency> obsoletes;
    std::list<YUMDependency> freshen;
    std::list<YUMDependency> requires;


*/
      }
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
