## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base detail

## ##################################################

include_HEADERS = \
	ResKind.h	\
	ResName.h	\
	ResArch.h	\
	ResEdition.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	ResKind.cc	\
	ResName.cc	\
	ResArch.cc	\
	ResEdition.cc

lib@PACKAGE@_la_LDFLAGS =	@LIB_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				detail/lib@PACKAGE@_detail.la

## ##################################################
