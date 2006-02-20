/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLPatternImpl.cc
 *
*/
#include "zypp/target/store/xml/XMLPatternImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLPatternImpl::XMLPatternImpl
    //	METHOD TYPE : Ctor
    //
    XMLPatternImpl::XMLPatternImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLPatternImpl::~XMLPatternImpl
    //	METHOD TYPE : Dtor
    //
    XMLPatternImpl::~XMLPatternImpl()
    {}

      bool XMLPatternImpl::userVisible() const {
        return _user_visible;
      }

      TranslatedText XMLPatternImpl::summary() const
      { return _summary; }

      TranslatedText XMLPatternImpl::description() const
      { return _description; }

      Text XMLPatternImpl::insnotify() const
      { return ResObjectImplIf::insnotify(); }

      Text XMLPatternImpl::delnotify() const
      { return ResObjectImplIf::delnotify(); }

      bool XMLPatternImpl::providesSources() const
      { return ResObjectImplIf::providesSources(); }

      Label XMLPatternImpl::instSrcLabel() const
      { return ResObjectImplIf::instSrcLabel(); }

      Vendor XMLPatternImpl::instSrcVendor() const
      { return ResObjectImplIf::instSrcVendor(); }

      ByteCount XMLPatternImpl::size() const
      { return ResObjectImplIf::size(); }

      /** */
      bool XMLPatternImpl::isDefault() const
      { return _default; }
      /** */
      TranslatedText XMLPatternImpl::category() const
      { return _category; }
      /** */
      Pathname XMLPatternImpl::icon() const
      { return _icon; }
      /** */
      Pathname XMLPatternImpl::script() const
      { return _script; }

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
