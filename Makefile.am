## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list 'devel' in subdirs. It's developers playgound
# and must not prevent anything from building.

if BUILD_ZMD
  SUB_ZMD=zmd
else
  SUB_ZMD=
endif
SUBDIRS = po 	zypp testsuite $(SUB_ZMD)

## ##################################################

.PHONY: package

package: dist


#ACLOCAL_AMFLAGS = -I m4
ACLOCAL_AMFLAGS =

#EXTRA_DIST = config.rpath mkinstalldirs
