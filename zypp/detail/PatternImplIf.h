/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PatternImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PATTERNIMPLIF_H
#define ZYPP_DETAIL_PATTERNIMPLIF_H

#include "zypp/detail/ResObjectImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Pattern;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PatternImplIf
    //
    /** Abstact Pattern implementation interface.
    */
    class PatternImplIf : public ResObjectImplIf
    {
    public:
      typedef Pattern ResType;

    public:
#if 0
      virtual std::string summary( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> description( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> insnotify( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delnotify( const LangCode & lang = LangCode("") ) const;
      virtual ByteCount size() const;
      virtual bool providesSources() const;
      virtual std::string instSrcLabel() const;
      virtual Vendor instSrcVendor() const;
      virtual unsigned instSrcRank() const;
      virtual std::string category() const;
      virtual bool visible() const;
      virtual std::list<std::string> suggests() const;
      virtual std::list<PMPatternPtr> suggests_ptrs() const;
      virtual std::list<std::string> recommends() const;
      virtual std::list<PMPatternPtr> recommends_ptrs() const;
      virtual std::list<std::string> inspacks( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delpacks( const LangCode & lang = LangCode("") ) const;
      virtual PM::LocaleSet supportedLocales() const;
      virtual std::set<PMSelectablePtr> pureInspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> inspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> delpacks_ptrs( const LangCode & lang ) const;
      virtual ByteCount archivesize() const;
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
#endif // ZYPP_DETAIL_PATTERNIMPLIF_H
