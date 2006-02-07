/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMMessageImpl.cc
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

      /** Default ctor
      */
      YUMMessageImpl::YUMMessageImpl(
	Source_Ref source_r,
	const zypp::parser::yum::YUMPatchMessage & parsed
      )
      : _source(source_r)
      {
	_text = parsed.text;
      }
      /** Get the text of the message */
      TranslatedText YUMMessageImpl::text() const {
	return _text;
      }
      TranslatedText YUMMessageImpl::summary() const
      { return ResObjectImplIf::summary(); }

      TranslatedText YUMMessageImpl::description() const
      { return ResObjectImplIf::description(); }

      Text YUMMessageImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMMessageImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      bool YUMMessageImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMMessageImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMMessageImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

      Source_Ref YUMMessageImpl::source() const
      { return _source; }


    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
