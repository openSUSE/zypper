## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list 'devel' in subdirs. It's developers playgound
# and must not prevent anything from building.

SUBDIRS = zypp testsuite po doc

## ##################################################

.PHONY: package

package: dist


#ACLOCAL_AMFLAGS = -I m4
ACLOCAL_AMFLAGS =

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libzypp.pc

EXTRA_DIST = mkinstalldirs
#config.rpath
