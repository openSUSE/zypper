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
      /** Ctor */
      SelectionImplIf() : ResObjectImplIf ()
      {}
      /** Dtor. Makes this an abstract class. */
      virtual ~SelectionImplIf() = 0;

    public:

        /** */
        virtual const TranslatedText & summary() const = 0;

        /** */
        virtual const TranslatedText & description() const = 0;

        /** */
	virtual Label category() const = 0;

        /** */
	virtual bool visible() const = 0;
    
        /** */
	virtual Label order() const = 0;
#if 0
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
#endif
    };
    ///////////////////////////////////////////////////////////////////

    /* Implementation of pure virtual dtor is required! */
    inline SelectionImplIf::~SelectionImplIf()
    {}


    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPLIF_H
