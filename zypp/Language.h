/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Language.h
 *
*/
#ifndef ZYPP_LANGUAGE_H
#define ZYPP_LANGUAGE_H

#include "zypp/ResObject.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(Language);

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //  CLASS NAME : LanguageImplIf
    //
    /** Exposition only. */
    class LanguageImplIf : public ResObjectImplIf
    {
    public:
      typedef Language ResType;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Language
  //
  /** Language interface.
  */
  class Language : public ResObject
  {
  public:
    typedef detail::LanguageImplIf   Impl;
    typedef Language                 Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

    /** Installed Language instance. */
    static Ptr installedInstance( const Locale & locale_r );
    /** Available Language instance. */
    static Ptr availableInstance( const Locale & locale_r );

  protected:
    /** Ctor */
    Language( const NVRAD & nvrad_r );
    /** Dtor */
    virtual ~Language();

  private:
    /** Access implementation */
    virtual Impl & pimpl() = 0;
    /** Access implementation */
    virtual const Impl & pimpl() const = 0;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_LANGUAGE_H
