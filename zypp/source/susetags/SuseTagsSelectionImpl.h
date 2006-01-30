/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H
#define ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H

#include "zypp/detail/SelectionImplIf.h"

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
    //	CLASS NAME : SelectionImpl
    //
    /**
    */
    struct SuseTagsSelectionImpl : public zypp::detail::SelectionImplIf
    {
    public:
      SuseTagsSelectionImpl();
      virtual ~SuseTagsSelectionImpl();

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
      virtual std::list<PMSelectionPtr> suggests_ptrs() const;
      virtual std::list<PMSelectionPtr> recommends_ptrs() const;
      virtual std::list<std::string> inspacks( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delpacks( const LangCode & lang = LangCode("") ) const;
      virtual PM::LocaleSet supportedLocales() const;
      virtual std::set<PMSelectablePtr> pureInspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> inspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> delpacks_ptrs( const LangCode & lang ) const;
      virtual bool isBase() const;
      virtual PMError provideSelToInstall( Pathname & path_r ) const;
      */

      virtual Label summary() const;
      virtual Text description() const;
      virtual Label category() const;
      virtual bool visible() const;
      virtual Label order() const;

      std::map< std::string, std::string> _summary;
      std::string _description;
      std::string _parser_version;
      std::string _name;
      std::string _version;
      std::string _release;
      std::string _arch;
      std::string _order;
      std::string _category;
      bool _visible;

      std::set<std::string> _suggests;
      std::set<std::string> _recommends;
      std::set<std::string> _requires;
      std::set<std::string> _conflicts;
      std::set<std::string> _supported_locales;
      std::map< std::string, std::set<std::string> > _insnotify;
      std::map< std::string, std::set<std::string> > _delnotify;
      std::map< std::string, std::set<std::string> > _inspacks;
      std::map< std::string, std::set<std::string> > _delpacks;
      
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPL_H
