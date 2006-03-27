/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/XMLLanguageImpl.cc
 *
*/
#include "zypp/target/store/xml/XMLLanguageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace storage
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLLanguageImpl::XMLLanguageImpl
    //	METHOD TYPE : Ctor
    //
    XMLLanguageImpl::XMLLanguageImpl()
    : LanguageImplIf(TranslatedText())
#warning FIXME line above, was added just to compile
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : XMLLanguageImpl::~XMLLanguageImpl
    //	METHOD TYPE : Dtor
    //
    XMLLanguageImpl::~XMLLanguageImpl()
    {}

      TranslatedText XMLLanguageImpl::summary() const
      { return _summary; }

      TranslatedText XMLLanguageImpl::description() const
      { return _description; }

    /////////////////////////////////////////////////////////////////
  } // namespace storage
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
