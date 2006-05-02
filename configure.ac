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

CXXFLAGS="${CXXFLAGS} -g -O2 -Wall -Wformat"
AC_SYS_LARGEFILE

dnl ==================================================

docdir=\${datadir}/doc/packages
pkgdocdir=\${docdir}/$PACKAGE

AC_SUBST(docdir)
AC_SUBST(pkgdocdir)

zyppdir=${prefix}/share/$PACKAGE
localedir=${prefix}/share/locale
yumschemadir=${zyppdir}/schema/yum
xmlstoreschemadir=${zyppdir}/schema/xmlstore

AC_SUBST(zyppdir)
AC_SUBST(localedir)
AC_SUBST(yumschemadir)
AC_SUBST(xmlstoreschemadir)

AH_TEMPLATE([LOCALEDIR],[gettext localedir])
AC_DEFINE_UNQUOTED([LOCALEDIR], ["${localedir}"])
AH_TEMPLATE([TEXTDOMAIN],[gettext textdomain])
AC_DEFINE_UNQUOTED([TEXTDOMAIN], ["$PACKAGE"])

dnl ==================================================
dnl checks for programs

dnl   check for c++ compiler and tools
AC_PROG_CXX
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AM_PROG_LIBTOOL

dnl   do AC_CHECK_* tests using c++ (e.g. boost header)
AC_LANG([C++])

AC_PATH_PROG(DOCGEN, doxygen)
if test -z "$DOCGEN" ; then
    AC_MSG_ERROR(doxygen is missing; please install doxygen.)
fi

AC_PATH_PROG(HAVE_DOT, dot)
if test -z "$HAVE_DOT" ; then
    AC_MSG_WARN(dot not found; you should install graphviz.)
    HAVE_DOT=NO
else
    HAVE_DOT=YES
fi

AC_PATH_PROG(XGETTEXT, xgettext)
if test -z "$XGETTEXT" ; then
    AC_MSG_ERROR(xgettext is missing; please install gettext-devel.)
fi

AC_PATH_PROG(MYRUNTEST, runtest)
if test -z "$MYRUNTEST" ; then
    AC_MSG_ERROR(runtest is missing; please install dejagnu.)
fi

dnl ==================================================
dnl
dnl Switches during develpoment only. They disable code using
dnl certain libraries (e.g. hal ) which may be temporarily
dnl unavailable or broken. This is just to allow building and testing
dnl the remaining parts of zypp.
dnl
AC_ARG_ENABLE(faked-hal, [  --enable-faked-hal   do not use libhal (DEVELOPMENT ONLY)],
  devel_faked_hal=yes, devel_faked_hal=no)

dnl ==================================================
dnl checks for libraries
AC_CHECK_LIB([z], [gzopen],
        [:],
        AC_MSG_ERROR([libz not found. please install zlib-devel]))
AC_CHECK_LIB([xml2], [xmlNewTextReader],
        [:],
        AC_MSG_ERROR([libxml2 not found. please install libxml2-devel]))

dnl ==================================================

if test "x$devel_faked_hal" = "xyes"; then
 HAL_CFLAGS=-DFAKE_HAL
else
 dnl checks for pkgconfig modules
 hal_module="hal >= 0.5.4, hal-storage >= 0.5.4"
 dbus_module="dbus-1 >= 0.33"
 hal_modules="glib-2.0 >= 2.6.0, gobject-2.0 >= 2.6.0, dbus-glib-1 >= 0.33, $dbus_module, $hal_module"
 PKG_CHECK_MODULES(HAL, [$hal_modules])
fi
AC_SUBST(HAL_CFLAGS)
AC_SUBST(HAL_LDADD)
AC_SUBST(HAL_LIBS)

dnl ==================================================
PKG_CHECK_MODULES(CURL, [libcurl])
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

dnl ==================================================
PKG_CHECK_MODULES(BLKID, [blkid])
AC_SUBST(BLKID_CFLAGS)
AC_SUBST(BLKID_LIBS)

dnl ==================================================
dnl checks for header files
AC_CHECK_HEADERS([boost/scoped_ptr.hpp],
        [],
        AC_MSG_ERROR([boost header not found. please install boost-devel]))
AC_CHECK_HEADERS([curl/curl.h],
        [],
        AC_MSG_ERROR([libcurl header not found. please install curl-devel.]))
AC_CHECK_HEADERS([blkid/blkid.h],
        [],
        AC_MSG_ERROR([libblkid header not found. please install e2fsprogs-devel.]))
AC_CHECK_HEADERS([rpm/rpmlib.h],
        [],
        AC_MSG_ERROR([rpmlib header not found. please install rpm-devel.]))
AC_CHECK_HEADERS([popt.h],
        [],
        AC_MSG_ERROR([popt header not found. please install popt-devel.]))

dnl internationalization macros
AM_GNU_GETTEXT

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
AC_OUTPUT(	 po/Makefile.in\
	Makefile			\
	libzypp.pc			\
	devel/Makefile			\
	devel/genclass			\
	devel/devel.jsrain/Makefile	\
  devel/devel.dmacvicar/Makefile \
	devel/devel.jsrain/yum/Makefile	\
	devel/devel.ma/Makefile		\
	devel/devel.kkaempf/Makefile	\
	doc/Makefile			\
	doc/autodoc/Makefile		\
	doc/autodoc/Doxyfile		\
	testsuite/Makefile		\
	testsuite/utils/Makefile	\
	testsuite/zypp/Makefile		\
	testsuite/zypp/tests/Makefile	\
	testsuite/solver/Makefile	\
	testsuite/solver/src/Makefile	\
	testsuite/solver/src/helix/Makefile	\
	testsuite/solver/single.out/Makefile	\
	testsuite/source/Makefile	\
	testsuite/source/tests/Makefile	\
	testsuite/parser/Makefile	\
	testsuite/parser/tests/Makefile	\
	testsuite/target/Makefile	\
	testsuite/target/tests/Makefile	\
	testsuite/media/Makefile	\
	testsuite/media/tests/Makefile	\
	zypp/Makefile			\
	zypp/base/Makefile		\
	zypp/thread/Makefile		\
	zypp/detail/Makefile		\
	zypp/capability/Makefile	\
	zypp/parser/Makefile		\
	zypp/parser/yum/Makefile	\
	zypp/parser/yum/schema/Makefile \
  zypp/parser/tagfile/Makefile  \
	zypp/parser/xmlstore/Makefile   \
  zypp/parser/xmlstore/schema/Makefile  \
	zypp/pool/Makefile		\
	zypp/solver/Makefile		\
	zypp/solver/detail/Makefile	\
	zypp/source/Makefile		\
	zypp/source/susetags/Makefile	\
	zypp/source/yum/Makefile	\
	zypp/media/Makefile		\
	zypp/media/proxyinfo/Makefile	\
	zypp/ui/Makefile		\
	zypp/url/Makefile		\
	zypp/target/Makefile		\
	zypp/target/rpm/Makefile	\
	zypp/target/store/Makefile	\
	zypp/target/store/xml/Makefile  \
	zypp/target/hal/Makefile	\
	zypp/target/modalias/Makefile	\
	zypp/zypp_detail/Makefile
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
if test "x$devel_faked_hal" = "xyes"; then
  echo "**WARNIG: 	faked_hal"
fi
echo

