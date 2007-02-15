dnl Process this file with autoconf to produce a configure script.
dnl ==================================================
m4_include([VERSION])
dnl AC_INIT(name, version)
AC_INIT([zypper], [ZYPPER_MAJOR.ZYPPER_MINOR.ZYPPER_PATCH])
dnl ==================================================
AC_CONFIG_SRCDIR(src/Makefile.am)

AC_PREFIX_DEFAULT(/usr)
AC_CANONICAL_TARGET
dnl enable renaming the resulting program
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(foreign dist-bzip2 no-dist-gzip tar-ustar)
AM_CONFIG_HEADER(config.h)

dnl ==================================================

docdir=\${datadir}/doc/packages
pkgdocdir=\${docdir}/$PACKAGE

AC_SUBST(docdir)
AC_SUBST(pkgdocdir)

localedir=${prefix}/share/locale

AC_SUBST(localedir)

AH_TEMPLATE([LOCALEDIR],[gettext localedir])
AC_DEFINE_UNQUOTED([LOCALEDIR], ["${localedir}"])
AH_TEMPLATE([TEXTDOMAIN],[gettext textdomain])
AC_DEFINE_UNQUOTED([TEXTDOMAIN], ["$PACKAGE"])

dnl ==================================================
dnl checks for programs

dnl invoked by AM_GNU_GETTEXT later. must be applied consistently.
AC_GNU_SOURCE

dnl   check for c++ compiler and tools
AC_PROG_CXX
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl   do AC_CHECK_* tests using c++ (e.g. boost header)
AC_LANG([C++])

AC_PATH_PROG(XGETTEXT, xgettext)
if test -z "$XGETTEXT" ; then
    AC_MSG_ERROR(xgettext is missing; please install gettext-devel.)
fi

dnl ==================================================
dnl checks for libraries

AC_ARG_VAR([PKG_CONFIG_PATH], [where to search for pkg-config files])
PKG_CHECK_MODULES(ZYPP, [libzypp >= 1.0.0])

case "$ZYPP_CFLAGS" in
  *LIBZYPP_1xx*) checkpatches=false ;;
  *) checkpatches=true ;;
esac
AM_CONDITIONAL([BUILD_CHECKPATCHES], [$checkpatches])

dnl ==================================================
dnl checks for header files
AC_CHECK_HEADERS([boost/scoped_ptr.hpp],
        [],
        AC_MSG_ERROR([boost header not found. please install boost-devel]))

dnl internationalization macros
AM_GNU_GETTEXT([external])

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
AC_OUTPUT( po/Makefile.in po/Makefile.in
	Makefile			\
	zypper.spec			\
	src/Makefile			\
	doc/Makefile
)
dnl ==================================================

echo "
Configuration:

  PACKAGE:              ${PACKAGE}
  VERSION:              ${VERSION}

  COMPILER:             ${CXX}
  CXXFLAGS:             ${CXXFLAGS}

  PREFIX:               ${prefix}
  LIBDIR:               ${libdir}

"
echo

