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
#include <sstream>
//#include "zypp/base/Logger.h"

#include "zypp/TranslatedText.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct TranslatedText::Private
  {
    std::map<LanguageCode, std::string> translations;
  };

  std::string TranslatedText::text( const LanguageCode &lang ) const
  {
    return d->translations[lang];
  }
  
  LanguageCode TranslatedText::detectLanguage() const
  {
     return LanguageCode();
  }

  void TranslatedText::setText( const std::string &text, const LanguageCode &lang )
  {
    d->translations[lang] = text;
  }

  void TranslatedText::setText( const std::list<std::string> &text, const LanguageCode &lang )
  {
    std::stringstream strs(d->translations[lang]);
    //d->translations[lang].clear();
    std::list<std::string>::const_iterator it;
    for (it = text.begin(); it != text.end(); ++it)
    {
      strs << *it << std::endl;
      //d->translations[lang] = d->translations[lang] + *it;
      //d->translations[lang] = d->translations[lang] + std::endl;
    }
  }

  /*
  operator TranslatedText::string() const
  {
    return text();
  }
  */

  TranslatedText::TranslatedText(const std::string &text, const LanguageCode &lang)
  {
    d = new Private();
    setText(text, lang);
  }

  TranslatedText::TranslatedText(const std::list<std::string> &text, const LanguageCode &lang)
  {
    d = new Private();
    setText(text, lang);
  }

  TranslatedText::TranslatedText()
  {
    d = new Private;
  }

  TranslatedText::~TranslatedText()
  {
    delete d;
  }
    

  void TranslatedText::operator=(const std::string &text)
  {
    setText(text);
  }
    
  void TranslatedText::operator=(const std::list<std::string> &text)
  {
    setText(text);
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : TranslatedText::asString
  //	METHOD TYPE : std::string
  //
  std::string TranslatedText::asString() const
  {
    return std::string();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
