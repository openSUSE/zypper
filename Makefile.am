## Process this file with automake to produce Makefile.in
## ##################################################

# Do NOT list 'devel' in subdirs. It's developers playgound
# and must not prevent anything from building.

SUBDIRS = doc src po

## ##################################################

ACLOCAL_AMFLAGS = -I m4 
EXTRA_DIST = config.rpath

## ##################################################

.PHONY: package package-local check-up-to-date

package: 	check-up-to-date package-local

package-local:	dist-bzip2 @PACKAGE@.spec
	@if [ `bunzip2 -c $(distdir).tar.bz2 | wc --bytes` = 0 ] ; then \
		echo "Error: $(distdir).tar.bz2 is empty" ; \
		exit 1 ; \
	fi
	mkdir -p package
	rm -f package/*.tar.bz2
	rm -f package/*~
	rm -f package/*.bak
	rm -f package/*.auto
	mv $(distdir).tar.bz2 package/
	mv @PACKAGE@.spec package/
	$(INSTALL_DATA) -p $(srcdir)/package/@PACKAGE@.changes package/

check-up-to-date:
	cd $(srcdir) && ! LC_ALL=C svn status --show-updates --quiet | grep -v '^Status against revision'

