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
#ifndef ZYPP_DETAIL_SELECTIONIMPL_H
#define ZYPP_DETAIL_SELECTIONIMPL_H

#include <list>

#include "zypp/detail/ResolvableImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(SelectionImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SelectionImpl
    //
    /** */
    class SelectionImpl : public ResolvableImpl
    {
    public:
      /** */
      SelectionImpl( const ResName & name_r,
                     const Edition & edition_r,
                     const Arch & arch_r );
      /** Dtor */
      virtual ~SelectionImpl();

    public:
      /** */
      virtual std::string summary() const;
      /** */
      virtual std::list<std::string> description() const;
#if 0
      virtual std::string summary( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> description( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> insnotify( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delnotify( const LangCode & lang = LangCode("") ) const;
      virtual FSize size() const;
      virtual bool providesSources() const;
      virtual std::string instSrcLabel() const;
      virtual Vendor instSrcVendor() const;
      virtual unsigned instSrcRank() const;
      virtual std::string category() const;
      virtual bool visible() const;
      virtual std::list<std::string> suggests() const;
      virtual std::list<PMSelectionPtr> suggests_ptrs() const;
      virtual std::list<std::string> recommends() const;
      virtual std::list<PMSelectionPtr> recommends_ptrs() const;
      virtual std::list<std::string> inspacks( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delpacks( const LangCode & lang = LangCode("") ) const;
      virtual PM::LocaleSet supportedLocales() const;
      virtual std::set<PMSelectablePtr> pureInspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> inspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> delpacks_ptrs( const LangCode & lang ) const;
      virtual FSize archivesize() const;
      virtual std::string order() const;
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
#endif // ZYPP_DETAIL_SELECTIONIMPL_H
