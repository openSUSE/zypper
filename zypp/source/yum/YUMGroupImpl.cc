/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMGroupImpl.cc
 *
*/

#include "zypp/source/yum/YUMGroupImpl.h"
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
      //        CLASS NAME : YUMGroupImpl
      //
      ///////////////////////////////////////////////////////////////////

      /** Default ctor
      */
      YUMGroupImpl::YUMGroupImpl(
	Source_Ref source_r,
	const zypp::parser::yum::YUMGroupData & parsed
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
	for (std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin();
	    it != parsed.grouplist.end();
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


// to name        std::string groupId;
// as _summary        std::list<multilang> name;
// _description
      }
      
      YUMGroupImpl::~YUMGroupImpl()
      {}

      /** Is to be visible for user? */
      bool YUMGroupImpl::visible() const {
	return _user_visible;
      }
      CapSet YUMGroupImpl::optionalReq() const
      { return _optional_req; }

      CapSet YUMGroupImpl::defaultReq() const
      { return _default_req; }

      const TranslatedText & YUMGroupImpl::summary() const
      { return ResObjectImplIf::summary(); }

      const TranslatedText & YUMGroupImpl::description() const
      { return ResObjectImplIf::description(); }

      Text YUMGroupImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text YUMGroupImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      bool YUMGroupImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label YUMGroupImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor YUMGroupImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

      ByteCount YUMGroupImpl::size() const
      { return ResObjectImplIf::size(); }

      Label YUMGroupImpl::order() const {
#warning Ordering support in YUM?
	return Label("0");
      }

     Label YUMGroupImpl::category() const {
#warning Category support in YUM missing!
	return Label("base");
      }

      Source_Ref YUMGroupImpl::source() const
      { return _source; }

 
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
