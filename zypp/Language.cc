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
#include "zypp/TranslatedText.h"
#include "zypp/CapFactory.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : LanguageImpl
    //
    /** Exposition only. */
    class LanguageImpl : public LanguageImplIf
    {
    public:
      LanguageImpl( const Locale & locale_r )
      : _locale( locale_r )
      {}

    public:
      virtual TranslatedText summary() const
      { return TranslatedText( _locale.name() ); }

      virtual TranslatedText description() const
      { return summary(); }

    private:
       Locale _locale;
    };
    ///////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////


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
        detail::ResImplTraits<detail::LanguageImpl>::Ptr langImpl( new detail::LanguageImpl( locale_r ) );
        NVRAD dataCollect( locale_r.code() );

        if ( locale_r.country().hasCode() )
          {
            // Recommend fallback Language
            Locale fallback( locale_r.fallback() );
            if ( fallback != Locale::noCode )
              dataCollect[Dep::RECOMMENDS].insert( CapFactory().parse( ResTraits<Language>::kind, fallback.code() ) );
          }

        ret = _ptrmap[locale_r] = detail::makeResolvableFromImpl( dataCollect, langImpl );
      }
    return ret;
  }

  Language::Ptr Language::availableInstance( const Locale & locale_r )
  {
    static std::map<Locale,Ptr> _ptrmap;
    Ptr ret( _ptrmap[locale_r] );
    if ( ! ret )
      {
        detail::ResImplTraits<detail::LanguageImpl>::Ptr langImpl( new detail::LanguageImpl( locale_r ) );
        NVRAD dataCollect( locale_r.code() );

        if ( locale_r.country().hasCode() )
          {
            // Recommend fallback Language
            Locale fallback( locale_r.fallback() );
            if ( fallback != Locale::noCode )
              dataCollect[Dep::RECOMMENDS].insert( CapFactory().parse( ResTraits<Language>::kind, fallback.code() ) );
          }

        ret = _ptrmap[locale_r] = detail::makeResolvableFromImpl( dataCollect, langImpl );
      }
    return ret;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
