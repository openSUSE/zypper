## Process this file with automake to produce Makefile.in
## ##################################################

htmldir =	@pkgdocdir@/autodocs

html_DATA =	@PACKAGE@.doxytag

# lots of files -> xargs
# (-hook runs _after_ the regular targets which means the dir already exists)
install-data-hook:
	find html -mindepth 1 | xargs $(INSTALL_DATA) --target-directory=$(DESTDIR)$(htmldir)

## ##################################################

# touch Doxyfile (or just make clean) to remake the html docs
@PACKAGE@.doxytag: Doxyfile
	@DOCGEN@

## ##################################################

CLEANFILES = $(html_DATA)

clean-local:
	-rm -rf html

## ##################################################
