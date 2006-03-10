/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/TranslatedText.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp/base/String.h"
#include "zypp/TranslatedText.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : TranslatedText::Impl
  //
  /** TranslatedText implementation. */
  struct TranslatedText::Impl
  {
    Impl()
    {}

    Impl(const std::string &text, const Locale &lang)
    { setText(text, lang); }

    Impl(const std::list<std::string> &text, const Locale &lang)
    { setText(text, lang); }

    std::string text( const Locale &lang = Locale() ) const
    {
      // if there is no translation for this
      if ( translations[lang].empty() )
      {
          // first, detect the locale
          ZYpp::Ptr z = getZYpp();
          Locale detected_lang( z->getTextLocale() );
          if ( translations[detected_lang].empty() )
          {
            Locale fallback_locale = detected_lang.fallback();
            while ( fallback_locale != Locale() )
            {
              if ( ! translations[fallback_locale].empty() )
                return translations[fallback_locale];

              fallback_locale = fallback_locale.fallback();
            }
            // we gave up, there are no translations with fallbacks
            // last try, emtpy locale
            Locale empty_locale;
            if ( ! translations[empty_locale].empty() )
              return translations[empty_locale];
            else
              return std::string();
          }
          else
          {
            return translations[detected_lang];
          }
      }
      else
      {
        return translations[lang]; 
      }
    }

    std::set<Locale> locales() const
    {
      std::set<Locale> lcls;
      for(std::map<Locale, std::string>::const_iterator it = translations.begin(); it != translations.end(); ++it)
      {
        lcls.insert((*it).first);
      }
      return lcls;
    }

    void setText( const std::string &text, const Locale &lang)
    { translations[lang] = text; }

    void setText( const std::list<std::string> &text, const Locale &lang)
    { translations[lang] = str::join( text, "\n" ); }

    /** \todo Do it by accessing the global ZYpp. */
    Locale detectLanguage() const
    {
      return Locale();
    }

  private:
    mutable std::map<Locale, std::string> translations;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : TranslatedText
  //
  ///////////////////////////////////////////////////////////////////

  const TranslatedText TranslatedText::notext;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : TranslatedText::TranslatedText
  //	METHOD TYPE : Ctor
  //
  TranslatedText::TranslatedText()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : TranslatedText::TranslatedText
  //	METHOD TYPE : Ctor
  //
  TranslatedText::TranslatedText( const std::string &text,
                                  const Locale &lang )
  : _pimpl( new Impl(text, lang) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : TranslatedText::TranslatedText
  //	METHOD TYPE : Ctor
  //
  TranslatedText::TranslatedText( const std::list<std::string> &text,
                                  const Locale &lang )
  : _pimpl( new Impl(text, lang) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : TranslatedText::~TranslatedText
  //	METHOD TYPE : Dtor
  //
  TranslatedText::~TranslatedText()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////

  std::string TranslatedText::text( const Locale &lang ) const
  { return _pimpl->text( lang ); }

  void TranslatedText::setText( const std::string &text, const Locale &lang )
  { _pimpl->setText( text, lang ); }

  std::set<Locale> TranslatedText::locales() const
  {
    return _pimpl->locales();
  }

  void TranslatedText::setText( const std::list<std::string> &text, const Locale &lang )
  { _pimpl->setText( text, lang ); }

  Locale TranslatedText::detectLanguage() const
  { return _pimpl->detectLanguage(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
