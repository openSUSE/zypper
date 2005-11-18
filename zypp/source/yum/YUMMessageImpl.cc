/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/MessageImpl.cc
 *
*/

#include "zypp/source/yum/YUMMessageImpl.h"

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
      //        CLASS NAME : YUMMessageImpl
      //
      ///////////////////////////////////////////////////////////////////
  
      /** Default ctor */
      YUMMessageImpl::YUMMessageImpl(
	const zypp::parser::yum::YUMPatchMessage & parsed
      )
#warning MA cannot be constructed that way
#if 0
      : MessageImpl(
	parsed.name,
	Edition(parsed.ver, parsed.rel, strtol(parsed.epoch.c_str(), NULL, 10)),
	Arch("noarch")
      )
#endif
      {
	_text = parsed.text;
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
