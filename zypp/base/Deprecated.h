/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Deprecated.h
 *  \brief	Provides the ZYPP_DEPRECATED macro.
 */

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

