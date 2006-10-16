/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/xml/XMLPatternImpl.cc
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
