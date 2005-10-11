dnl Process this file with autoconf to produce a configure script.
dnl ==================================================

AC_INIT(zypp,0.0.0)
AC_CONFIG_SRCDIR(zypp/Makefile.am)

AC_PREFIX_DEFAULT(/usr)
AC_CANONICAL_TARGET
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(foreign dist-bzip2 no-dist-gzip tar-ustar)
AM_CONFIG_HEADER(config.h)

dnl ==================================================
dnl Versioning
dnl ==========
dnl
dnl CURRENT    The most recent interface number that this library implements.
dnl
dnl REVISION   The implementation number of the current interface.
dnl
dnl AGE        The difference between the newest and oldest interfaces that this
dnl            library implements.
dnl            In other words, the library implements all the interface numbers
dnl            in the range from number 'CURRENT - AGE' to 'CURRENT'.
dnl
dnl 1) Start with version information of `0:0:0' for each libtool library.
dnl
dnl 2) Update the version information only immediately before a public release
dnl    of your software. More frequent updates are unnecessary, and only guarantee
dnl    that the current interface number gets larger faster.
dnl
dnl 3) If the library source code has changed at all since the last update,
dnl    then increment revision (`c:r:a' becomes `c:r+1:a').
dnl
dnl 4) If any interfaces have been added, removed, or changed since the last
dnl    update, increment current, and set revision to 0.
dnl
dnl 5) If any interfaces have been added since the last public release, then
dnl    increment age.
dnl
dnl 6) If any interfaces have been removed since the last public release, then
dnl    set age to 0.
dnl

LIBZYPP_CURRENT=0
LIBZYPP_REVISION=0
LIBZYPP_AGE=0

LIBZYPP_VERSION=$LIBZYPP_CURRENT.$LIBZYPP_REVISION.$LIBZYPP_AGE
AC_SUBST(LIBZYPP_VERSION)

LIBZYPP_VERSION_INFO="-version-info $LIBZYPP_CURRENT:$LIBZYPP_REVISION:$LIBZYPP_AGE"
AC_SUBST(LIBZYPP_VERSION_INFO)

dnl ==================================================

CXXFLAGS="-g -O2 -Wall -Wformat"

dnl ==================================================

docdir=\${datadir}/doc/packages
pkgdocdir=\${docdir}/$PACKAGE

AC_SUBST(docdir)
AC_SUBST(pkgdocdir)

dnl ==================================================
dnl checks for programs
AC_PROG_CXX
AM_PROG_LIBTOOL
AC_PATH_PROG(DOCGEN, doxygen, doxygen)
if test -z "$DOCGEN" ; then
    AC_MSG_ERROR(doxygen is missing; please install doxygen.)
fi
AC_PATH_PROG(XGETTEXT, xgettext)
if test -z "$XGETTEXT" ; then
    AC_MSG_ERROR(xgettext is missing; please install gettext-devel.)
fi
dnl ==================================================
dnl checks for libraries
dnl ==================================================
dnl checks for header files
dnl ==================================================
dnl checks for typedefs
dnl ==================================================
dnl checks for structures
dnl ==================================================
dnl checks for compiler characteristics
dnl ==================================================
dnl checks for library functions
dnl ==================================================
dnl checks for system services
dnl ==================================================
dnl AC_CONFIG_FILES([FILE...])
AC_OUTPUT(	\
	Makefile		\
	doc/Makefile		\
	doc/autodoc/Makefile	\
	doc/autodoc/Doxyfile	\
	test/Makefile		\
	test/genclass		\
	zypp/Makefile		\
	zypp/base/Makefile	\
	zypp/detail/Makefile
)
dnl ==================================================

echo "
Configuration:

  PACKAGE:              ${PACKAGE}
  VERSION:              ${VERSION}
  LIBVERSION:           lib${PACKAGE}.so.${LIBZYPP_VERSION}

  COMPILER:             ${CXX}
  CXXFLAGS:          	${CXXFLAGS}

  PREFIX:               ${prefix}
  LIBDIR:               ${libdir}
"
