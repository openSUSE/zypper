/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/APIConfig.h
 *  \brief	Provides API related macros.
 */
#ifndef ZYPP_APICONFIG_H
#define ZYPP_APICONFIG_H

#include "zypp/base/Easy.h"	// some macros used almost everywhere

/**
 * Generic helper definitions for shared library support.
 *
 * \see e.g. http://gcc.gnu.org/wiki/Visibility
 * \code
 *   extern "C" ZYPP_API void function(int a);
 *   class ZYPP_API SomeClass
 *   {
 *      int c;
 *      ZYPP_LOCAL void privateMethod();  // Only for use within this DSO
 *   public:
 *      Person(int _c) : c(_c) { }
 *      static void foo(int a);
 *   };
 * \endcode
};*/
#if __GNUC__ >= 4
  #define ZYPP_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
  #define ZYPP_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
  #define ZYPP_HELPER_DLL_EXPORT
  #define ZYPP_HELPER_DLL_LOCAL
#endif

#ifdef ZYPP_DLL	//defined if zypp is compiled as DLL
  #define ZYPP_API	ZYPP_HELPER_DLL_EXPORT
  #define ZYPP_LOCAL	ZYPP_HELPER_DLL_LOCAL
#else
  #define ZYPP_API
  #define ZYPP_LOCAL
#endif

/**
 * The ZYPP_DEPRECATED macro can be used to trigger compile-time warnings
 * with gcc >= 3.2 when deprecated functions are used.
 *
 * For non-inline functions, the macro is used at the very end of the
 * function declaration, right before the semicolon, unless it's pure
 * virtual:
 *
 * int deprecatedFunc() const ZYPP_DEPRECATED;
 * virtual int deprecatedPureVirtualFunc() const ZYPP_DEPRECATED = 0;
 *
 * Functions which are implemented inline are handled differently:
 * the ZYPP_DEPRECATED macro is used at the front, right before the
 * return type, but after "static" or "virtual":
 *
 * ZYPP_DEPRECATED void deprecatedFuncA() { .. }
 * virtual ZYPP_DEPRECATED int deprecatedFuncB() { .. }
 * static  ZYPP_DEPRECATED bool deprecatedFuncC() { .. }
 *
 * You can also mark whole structs or classes as deprecated, by inserting
 * the ZYPP_DEPRECATED macro after the struct/class keyword, but before
 * the name of the struct/class:
 *
 * class ZYPP_DEPRECATED DeprecatedClass { };
 * struct ZYPP_DEPRECATED DeprecatedStruct { };
 *
 * However, deprecating a struct/class doesn't create a warning for gcc
 * versions <= 3.3 (haven't tried 3.4 yet).  If you want to deprecate a class,
 * also deprecate all member functions as well (which will cause warnings).
 *
 */
#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
  #ifndef ZYPP_DEPRECATED
  #define ZYPP_DEPRECATED __attribute__ ((deprecated))
  #endif
#else
  #ifndef ZYPP_DEPRECATED
  #define ZYPP_DEPRECATED
  #endif
#endif

#endif //ZYPP_APICONFIG_H
