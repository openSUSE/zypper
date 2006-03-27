/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Language.cc
 *
*/
#include "zypp/Language.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(Language);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Language::Language
  //	METHOD TYPE : Ctor
  //
  Language::Language( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Language::~Language
  //	METHOD TYPE : Dtor
  //
  Language::~Language()
  {}

  Language::Ptr Language::installedInstance( const Locale & locale_r )
  {
    static std::map<Locale,Ptr> _ptrmap;
    Ptr ret( _ptrmap[locale_r] );
    if ( ! ret )
      {
        NVRAD dataCollect( locale_r.code() );
        detail::ResImplTraits<detail::LanguageImplIf>::Ptr langImpl;
        ret = _ptrmap[locale_r] = detail::makeResolvableAndImpl( dataCollect, langImpl );
      }
    return ret;
  }

  Language::Ptr Language::availableInstance( const Locale & locale_r )
  {
    static std::map<Locale,Ptr> _ptrmap;
    Ptr ret( _ptrmap[locale_r] );
    if ( ! ret )
      {
        NVRAD dataCollect( locale_r.code() );
        detail::ResImplTraits<detail::LanguageImplIf>::Ptr langImpl;
        ret = _ptrmap[locale_r] = detail::makeResolvableAndImpl( dataCollect, langImpl );
      }
    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
