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
	const zypp::parser::yum::YUMGroupData & parsed
      )
      : _user_visible(parsed.userVisible == "true")
      {
	for (std::list<PackageReq>::const_iterator it
			= parsed.packageList.begin();
             it != parsed.packageList.end();
	     it++)
        {
	  _pkgs_req.push_back(PkgReq(it->type, it->name, it->ver,
				     it->rel, it->epoch));
        }
	for (std::list<MetaPkg>::const_iterator it
			= parsed.grouplist.begin();
             it != parsed.grouplist.end();
	     it++)
        {
	  _groups_req.push_back(GroupReq(it->type, it->name));
        }
// to name        std::string groupId;
// as _summary        std::list<multilang> name;
// _description
      }
      /** Is to be visible for user? */
      bool YUMGroupImpl::userVisible() const {
	return _user_visible;
      }

      std::list<YUMGroupImpl::GroupReq> YUMGroupImpl::groupsReq() const
      { return _groups_req; }

      std::list<YUMGroupImpl::PkgReq> YUMGroupImpl::pkgsReq() const
      { return _pkgs_req; }

      Label YUMGroupImpl::summary() const
      { return ResObjectImplIf::summary(); }

      Text YUMGroupImpl::description() const
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


    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
