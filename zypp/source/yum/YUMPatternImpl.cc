/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMPatternImpl.cc
 *
*/

#include "zypp/source/yum/YUMPatternImpl.h"
#include "zypp/CapFactory.h"

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
      //        CLASS NAME : YUMPatternImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      YUMPatternImpl::YUMPatternImpl(
	Source_Ref source_r,
	const zypp::parser::yum::YUMPatternData & parsed
      )
      : _user_visible(parsed.userVisible == "true")
      , _source(source_r)
      {
	CapFactory _f;
	for (std::list<PackageReq>::const_iterator it = parsed.packageList.begin();
	  it != parsed.packageList.end();
	  it++)
	{
          Capability _cap = _f.parse(
            ResTraits<Package>::kind,
            it->name,
            Rel("EQ"),
            Edition(it->ver, it->rel, it->epoch)
          );
	  if (it->type == "default")
	  {
	    _default_req.insert(_cap);
	  }
	  else if (it->type == "optional")
	  {
	    _optional_req.insert(_cap);
	  } 
	}
	for (std::list<MetaPkg>::const_iterator it = parsed.patternlist.begin();
	    it != parsed.patternlist.end();
	    it++)
	{
          Capability _cap = _f.parse(
            ResTraits<Selection>::kind,
            it->name,
            Rel(),
            Edition()
          );
	  if (it->type == "default")
	  {
	    _default_req.insert(_cap);
	  }
	  else if (it->type == "optional")
	  {
	    _optional_req.insert(_cap);
	  } 
	}


// to name        std::string patternId;
// as _summary        std::list<multilang> name;
// _description
      }
      /** Is to be visible for user? */
      bool YUMPatternImpl::userVisible() const {
	return _user_visible;
      }
      CapSet YUMPatternImpl::optionalReq() const
      { return _optional_req; }

      CapSet YUMPatternImpl::defaultReq() const
      { return _default_req; }

      TranslatedText YUMPatternImpl::summary() const
      { return ResObjectImplIf::summary(); }

      TranslatedText YUMPatternImpl::description() const
      { return ResObjectImplIf::description(); }

      Text YUMPatternImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMPatternImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      bool YUMPatternImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMPatternImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMPatternImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

      ByteCount YUMPatternImpl::size() const
      { return ResObjectImplIf::size(); }

      Source_Ref YUMPatternImpl::source() const
      { return _source; }


    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
