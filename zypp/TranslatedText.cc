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
    { return translations[lang]; }

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

  void TranslatedText::setText( const std::list<std::string> &text, const Locale &lang )
  { _pimpl->setText( text, lang ); }

  Locale TranslatedText::detectLanguage() const
  { return _pimpl->detectLanguage(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
