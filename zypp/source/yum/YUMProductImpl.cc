/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMProductImpl.cc
 *
*/

#include "zypp/source/yum/YUMProductImpl.h"
#include "zypp/source/yum/YUMSourceImpl.h"
#include <zypp/CapFactory.h>
#include "zypp/parser/yum/YUMParserData.h"
#include <zypp/parser/yum/YUMParser.h>
#include "zypp/Package.h"
#include "zypp/Script.h"
#include "zypp/Message.h"
#include "zypp/base/Logger.h"


using namespace std;
using namespace zypp::detail;
using namespace zypp::parser::yum;

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
      //        CLASS NAME : YUMProductImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
       * \bug CANT BE CONSTUCTED THAT WAY ANYMORE
      */
      YUMProductImpl::YUMProductImpl(
	Source_Ref source_r,
	const zypp::parser::yum::YUMProductData & parsed
      )
      :	_category(parsed.type),
	_vendor(parsed.vendor),
//	_displayname(parsed.displayname),
//	_description(parsed.description)
	_source(source_r)
      {}

      std::string YUMProductImpl::category() const
      { return _category; }

      Label YUMProductImpl::vendor() const
      { return _vendor; }

      Label YUMProductImpl::displayName() const
      { return _displayname; }

      TranslatedText YUMProductImpl::summary() const
      { return ResObjectImplIf::summary(); }

      TranslatedText YUMProductImpl::description() const
      { return _description; }

      Text YUMProductImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMProductImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      ByteCount YUMProductImpl::size() const
      { return ResObjectImplIf::size(); }

      bool YUMProductImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMProductImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMProductImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

      Source_Ref YUMProductImpl::source() const
      { return _source; }
 
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
