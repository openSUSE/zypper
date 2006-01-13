## Process this file with automake to produce Makefile.in
## ##################################################

htmldir =	@pkgdocdir@/autodocs

html_DATA = 	@PACKAGE@.doxytag $(wildcard html/*)

## ##################################################

@PACKAGE@.doxytag: Doxyfile
	@DOCGEN@

## ##################################################

CLEANFILES = $(html_DATA)

clean-local:
	-rmdir html

## ##################################################
