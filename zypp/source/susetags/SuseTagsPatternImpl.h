/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatternImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_PATTERNIMPL_H
#define ZYPP_DETAIL_SUSETAGS_PATTERNIMPL_H

#include "zypp/detail/PatternImplIf.h"
#include "zypp/Source.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
  namespace susetags
  {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternImpl
    //
    /**
    */
    struct SuseTagsPatternImpl : public zypp::detail::PatternImplIf
    {
    public:
      SuseTagsPatternImpl();
      virtual ~SuseTagsPatternImpl();

      virtual TranslatedText summary() const;
      virtual TranslatedText description() const;
      virtual TranslatedText category() const;
      virtual bool userVisible() const;
      virtual Label order() const;
      virtual Pathname icon() const;
      virtual Source_Ref source() const;
      
      TranslatedText _summary;
      TranslatedText _description;
      Pathname _icon;
      std::string _parser_version;
      std::string _name;
      std::string _version;
      std::string _release;
      std::string _arch;
      std::string _order;
      TranslatedText _category;
      
      bool _visible;

      std::list<std::string> _suggests;
      std::list<std::string> _recommends;
      std::list<std::string> _requires;
      std::list<std::string> _conflicts;
      std::list<std::string> _provides;
      std::list<std::string> _obsoletes;
      std::list<std::string> _pkgsuggests;
      std::list<std::string> _pkgrecommends;
      std::list<std::string> _pkgrequires;
      std::list<std::string> _supported_locales;
      std::map< Locale, std::list<std::string> > _insnotify;
      std::map< Locale, std::list<std::string> > _delnotify;

      Source_Ref _source;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PATTERNIMPL_H
