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

      /*
      virtual std::list<std::string> suggests() const;
      virtual std::list<std::string> recommends() const;
      virtual std::list<std::string> insnotify( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delnotify( const LangCode & lang = LangCode("") ) const;
      virtual ByteCount size() const;
      virtual bool providesSources() const;
      virtual std::string instSrcLabel() const;
      virtual Vendor instSrcVendor() const;
      virtual unsigned instSrcRank() const;
      virtual std::list<PMPatternPtr> suggests_ptrs() const;
      virtual std::list<PMPatternPtr> recommends_ptrs() const;
      virtual std::list<std::string> inspacks( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delpacks( const LangCode & lang = LangCode("") ) const;
      virtual PM::LocaleSet supportedLocales() const;
      virtual std::set<PMSelectablePtr> pureInspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> inspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> delpacks_ptrs( const LangCode & lang ) const;
      virtual bool isBase() const;
      virtual PMError provideSelToInstall( Pathname & path_r ) const;
      */

      virtual TranslatedText summary() const;
      virtual TranslatedText description() const;
      virtual Label category() const;
      virtual bool visible() const;
      virtual Label order() const;
      virtual Pathname icon() const;
      
      TranslatedText _summary;
      TranslatedText _description;
      Pathname _icon;
      std::string _parser_version;
      std::string _name;
      std::string _version;
      std::string _release;
      std::string _arch;
      std::string _order;
      std::string _category;
      
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
