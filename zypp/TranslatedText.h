/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Date.h
 *
*/
#ifndef ZYPP_TRANSLATEDTEXT_H
#define ZYPP_TRANSLATEDTEXT_H

#include <list>
#include <map>
#include <string>
#include "zypp/LanguageCode.h"

using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : TranslatedText
  //
  /** Class that represent a text and multiple translations
  */
  class TranslatedText
  {
    friend std::ostream & operator<<( std::ostream & str, const TranslatedText & obj );

  public:
    /** Default ctor: 0 */
    static const TranslatedText notext;
    TranslatedText();
    ~TranslatedText();
    TranslatedText(const std::string &text, const LanguageCode &lang = LanguageCode());
    TranslatedText(const std::list<std::string> &text, const LanguageCode &lang = LanguageCode());
    void operator=(const std::string &text);
    void operator=(const std::list<std::string> &text);
    std::string asString() const;
    std::string text( const LanguageCode &lang = LanguageCode() ) const;
    //operator string() const;
    void setText( const std::string &text, const LanguageCode &lang = LanguageCode());
    void setText( const std::list<std::string> &text, const LanguageCode &lang = LanguageCode());
    LanguageCode detectLanguage() const;
  private:
    class Private;
    Private *d;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Date Stream output */
  inline std::ostream & operator<<( std::ostream & str, const TranslatedText & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TRANSLATED_H
