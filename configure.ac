dnl Process this file with autoconf to produce a configure script.
dnl ==================================================
m4_include([VERSION])
AC_INIT([zypp], [LIBZYPP_CURRENT.LIBZYPP_AGE.LIBZYPP_REVISION])
dnl ==================================================
AC_CONFIG_SRCDIR(zypp/Makefile.am)

AC_PREFIX_DEFAULT(/usr)
AC_CANONICAL_TARGET
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(foreign dist-bzip2 no-dist-gzip tar-ustar)
AM_CONFIG_HEADER(config.h)

dnl ==================================================

LIBZYPP_VERSION_INFO="-version-info LIBZYPP_CURRENT:LIBZYPP_REVISION:LIBZYPP_AGE"
AC_SUBST(LIBZYPP_VERSION_INFO)

dnl ==================================================

# libzypp compile time features/option tags,
# that can be used for consistence checks ...
ZYPP_FEATURES=""
AC_SUBST(ZYPP_FEATURES)

# libzypp cflags to add to pkg-config cflags
ZYPP_CFLAGS=""
AC_SUBST(ZYPP_CFLAGS)

dnl ==================================================

CXXFLAGS="${CXXFLAGS} -g -O2 -Wall -Wformat"
AC_SYS_LARGEFILE
if test "$enable_largefile" != no ; then
	enable_largefile=yes
	if test "$ac_cv_sys_file_offset_bits" != no; then
		LFS_CFLAGS="-D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
	fi
	if test "$ac_cv_sys_large_files" != no; then
		LFS_CFLAGS="-D_LARGE_FILES=$ac_cv_sys_large_files $LFS_CFLAGS"
	fi
	ZYPP_FEATURES="LFS $ZYPP_FEATURES"
	ZYPP_CFLAGS="$LFS_CFLAGS $ZYPP_CFLAGS"
fi
AC_SUBST(LFS_CFLAGS)
test -n "$LFS_CFLAGS" && CXXFLAGS="$CXXFLAGS $LFS_CFLAGS"

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
PKG_CHECK_MODULES(CURL, [libcurl])
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)

dnl ==================================================
PKG_CHECK_MODULES(BLKID, [blkid])
AC_SUBST(BLKID_CFLAGS)
AC_SUBST(BLKID_LIBS)

dnl ==================================================
PKG_CHECK_MODULES(IDN, [libidn])
AC_SUBST(IDN_CFLAGS)
AC_SUBST(IDN_LIBS)

dnl ==================================================
PKG_CHECK_MODULES(OPENSSL, [openssl])
AC_SUBST(OPENSSL_CFLAGS)
AC_SUBST(OPENSSL_LIBS)

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
AC_CHECK_HEADERS([idn-free.h],
        [],
        AC_MSG_ERROR([libidn header not found. please install libidn-devel.]))
AC_CHECK_HEADERS([openssl/evp.h],
        [],
        AC_MSG_ERROR([openssl header not found. please install openssl-devel.]))
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
	libzypp.spec			\
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

  FEATURES:             ${ZYPP_FEATURES}

  COMPILER:             ${CXX}
  CXXFLAGS:             ${CXXFLAGS}

  PREFIX:               ${prefix}
  LIBDIR:               ${libdir}

"
if test "x$devel_faked_hal" = "xyes"; then
  echo "**WARNIG: 	faked_hal"
fi
echo

