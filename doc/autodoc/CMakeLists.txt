INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES(  FILES  @PACKAGE@.doxytag )



#original Makefile.am contents follow:

### Process this file with automake to produce Makefile.in
### ##################################################
#
#htmldir =	@pkgdocdir@/autodocs
#
#html_DATA =	@PACKAGE@.doxytag
#
## lots of files -> xargs
## (-hook runs _after_ the regular targets which means the dir already exists)
#install-data-hook:
#	find html -mindepth 1 | xargs $(INSTALL_DATA) --target-directory=$(DESTDIR)$(htmldir)
#
### ##################################################
#
#@PACKAGE@.doxytag: Doxyfile
#	@DOCGEN@
#
## force remaking the html docs
#doc:
#	$(MAKE) -W Doxyfile
#
### ##################################################
#
#CLEANFILES = $(html_DATA)
#
#clean-local:
#	-rm -rf html
#
### ##################################################
