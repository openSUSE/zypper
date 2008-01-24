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
  //
  //	CLASS NAME : Language
  //
  /** Language interface.
  */
  class Language : public ResObject
  {
  public:
    typedef Language                 Self;
    typedef ResTraits<Self>          TraitsType;
    typedef TraitsType::PtrType      Ptr;
    typedef TraitsType::constPtrType constPtr;

    /** Installed Language instance. */
    static Ptr installedInstance( const Locale & locale_r );
    /** Available Language instance. */
    static Ptr availableInstance( const Locale & locale_r );

  protected:
    friend Ptr make<Self>( const sat::Solvable & solvable_r );
    /** Ctor */
    Language( const sat::Solvable & solvable_r );
    /** Dtor */
    virtual ~Language();
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_LANGUAGE_H
