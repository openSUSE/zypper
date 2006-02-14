/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_SELECTIONIMPLIF_H
#define ZYPP_DETAIL_SELECTIONIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/TranslatedText.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Selection;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SelectionImplIf
    //
    /** Abstact Selection implementation interface.
    */
    class SelectionImplIf : public ResObjectImplIf
    {
    public:
      typedef Selection ResType;

    public:

        /** */
        virtual TranslatedText summary() const PURE_VIRTUAL;

        /** */
        virtual TranslatedText description() const PURE_VIRTUAL;

        /** */
	virtual Label category() const PURE_VIRTUAL;

        /** */
	virtual bool visible() const PURE_VIRTUAL;

        /** */
	virtual Label order() const PURE_VIRTUAL;

        /** ui helper */
        virtual std::set<std::string> install_packages( const Locale & lang = Locale("") ) const;
      
#if 0
        // NOTE LangCode id zypp:Locale
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
#endif
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPLIF_H
