/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/LanguageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_LANGUAGEIMPLIF_H
#define ZYPP_DETAIL_LANGUAGEIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Language;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LanguageImplIf
    //
    /** Abstract Language implementation interface.
    */
    class LanguageImplIf : public ResObjectImplIf
    {
    public:
      typedef Language ResType;

      LanguageImplIf( const TranslatedText & summary )
	: _summary( summary )
      { }

    public:
      virtual TranslatedText summary() const;

    protected:
      TranslatedText _summary;

    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_LANGUAGEIMPLIF_H
