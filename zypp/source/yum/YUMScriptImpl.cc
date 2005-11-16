/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ScriptImpl.cc
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
    namespace YUM
    {
      IMPL_PTR_TYPE(YUMScriptImpl)
  
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMScriptImpl
      //
      ///////////////////////////////////////////////////////////////////
  
      /** Default ctor */
      YUMScriptImpl::YUMScriptImpl(
	const zypp::parser::YUM::YUMPatchScript & parsed
      )
      : ScriptImpl(
	parsed.name,
	Edition(parsed.ver, parsed.rel, strtol(parsed.epoch.c_str(), NULL, 10)),
	Arch("noarch")
      )
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
    } // namespace YUM 
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
