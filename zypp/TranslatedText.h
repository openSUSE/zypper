/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/TranslatedText.h
 *
*/
#ifndef ZYPP_TRANSLATEDTEXT_H
#define ZYPP_TRANSLATEDTEXT_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : TranslatedText
  //
  /** Class that represent a text and multiple translations.
  */
  class TranslatedText
  {
    friend std::ostream & operator<<( std::ostream & str, const TranslatedText & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    TranslatedText();
    /** Ctor \todo Make ctor it explicit */
    bool empty() const ;
    explicit
    TranslatedText(const std::string &text, const Locale &lang = Locale());
    /** Ctor. \todo Make ctor it explicit */
    explicit
    TranslatedText(const std::list<std::string> &text, const Locale &lang = Locale());
    /** Dtor */
    ~TranslatedText();

    /**  */
    static const TranslatedText notext;

  public:

    /** Synonym for \ref text */
    std::string asString( const Locale &lang = Locale() ) const
    { return text(lang); }

    std::string text( const Locale &lang = Locale() ) const;
    std::set<Locale> locales() const;

    void setText( const std::string &text, const Locale &lang = Locale());
    void setText( const std::list<std::string> &text, const Locale &lang = Locale());

    Locale detectLanguage() const;

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates TranslatedText Stream output */
  inline std::ostream & operator<<( std::ostream & str, const TranslatedText & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TRANSLATEDTEXT_H
