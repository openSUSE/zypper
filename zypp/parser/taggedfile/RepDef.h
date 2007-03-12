/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       RepDef.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Provides a set of macros to define data and pointer classes.
  See class RepPtrStore in Rep.h.

/-*/
#ifndef RepDef_h
#define RepDef_h

#include "zypp/parser/taggedfile/Rep.h"

///////////////////////////////////////////////////////////////////
//
// NOTE: Data classes must inherit class Rep:
//
// class CLNAME : public Rep {
//   REP_BODY(CLNAME);
//   ...
// };
//
///////////////////////////////////////////////////////////////////

#define REP_BODY(NAME)         GEN_REP_BODY( NAME, #NAME )

///////////////////////////////////////////////////////////////////

#define GEN_REP_BODY(CLNAME,STRNAME)			\
  CLNAME( const CLNAME & );            /* no copy */	\
  CLNAME & operator=(const CLNAME & ); /* no assign */	\
  public:						\
    virtual const char * repName() const 		\
      { return STRNAME; }				\
    virtual size_t mem_size () const { return sizeof (CLNAME); }\
  private:

///////////////////////////////////////////////////////////////////

#define DEFINE_BASE_POINTER(NAME)				\
  class NAME;							\
  typedef Ptr<NAME>      NAME##Ptr;				\
  typedef constPtr<NAME> const##NAME##Ptr;			\
  extern template class RepPtrStore<NAME>;			\
  extern template class RepPtrStore<const NAME>;

#define DEFINE_BASE_POINTER_IN_NAMESPACE(NS,NAME)		\
  namespace NS {                                                \
  class NAME;							\
  typedef Ptr<NAME>      NAME##Ptr;				\
  typedef constPtr<NAME> const##NAME##Ptr;			\
  }                                                             \
  extern template class RepPtrStore<NS::NAME>;			\
  extern template class RepPtrStore<const NS::NAME>;

#define DEFINE_DERIVED_POINTER(NAME,BASE)			\
  class NAME;							\
  typedef Ptr<NAME,BASE>      NAME##Ptr;			\
  typedef constPtr<NAME,BASE> const##NAME##Ptr;			\
  extern template class RepPtrStore<NAME,BASE>;			\
  extern template class RepPtrStore<const NAME,const BASE>;

#define DEFINE_DERIVED_POINTER_IN_NAMESPACE(NS,NAME,BASE)	\
  namespace NS {                                                \
  class NAME;							\
  typedef Ptr<NAME,BASE>      NAME##Ptr;			\
  typedef constPtr<NAME,BASE> const##NAME##Ptr;			\
  }                                                             \
  extern template class RepPtrStore<NS::NAME,BASE>;		\
  extern template class RepPtrStore<const NS::NAME,const BASE>;

///////////////////////////////////////////////////////////////////

#define IMPL_BASE_POINTER(NAME)					\
template class RepPtrStore<NAME>;				\
template class RepPtrStore<const NAME>;

#define IMPL_BASE_POINTER_IN_NAMESPACE(NS,NAME)			\
template class RepPtrStore<NS::NAME>;				\
template class RepPtrStore<const NS::NAME>;

#define IMPL_DERIVED_POINTER(NAME,BASE)				\
template class RepPtrStore<NAME,BASE>;				\
template class RepPtrStore<const NAME,const BASE>;

#define IMPL_DERIVED_POINTER_IN_NAMESPACE(NS,NAME,BASE)		\
template class RepPtrStore<NS::NAME,BASE>;			\
template class RepPtrStore<const NS::NAME,const BASE>;

///////////////////////////////////////////////////////////////////

#endif // RepDef_h
