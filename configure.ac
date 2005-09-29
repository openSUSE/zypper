dnl Process this file with autoconf to produce a configure script.
dnl ==================================================
AC_INIT(zypp/Makefile.am)
dnl ==================================================

PACKAGE=zypp

VERSION_MAJOR=0
VERSION_MINOR=0
VERSION_RELEASE=0
VERSION="$VERSION_MAJOR.$VERSION_MINOR.$VERSION_RELEASE"
LIB_VERSION_INFO="-version-info $VERSION_MAJOR:$VERSION_MINOR:$VERSION_RELEASE"

COMMONFLAGS="-g -O2 -Wall -Wformat"
CFLAGS="${COMMONFLAGS} -Wmissing-prototypes ${CFLAGS}"
CXXFLAGS="${COMMONFLAGS} ${CXXFLAGS}"

dnl ==================================================
AM_INIT_AUTOMAKE($PACKAGE,$VERSION)
AM_CONFIG_HEADER(config.h)
dnl ==================================================

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(LIB_VERSION_INFO)

dnl ==================================================
dnl checks for programs
AC_PROG_CC
AC_PROG_CXX
AM_PROG_LIBTOOL
AC_CHECK_PROG(DOCGEN, doxygen, doxygen)
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
	zypp/Makefile		\
	zypp/base/Makefile
)
