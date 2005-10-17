## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base detail

## ##################################################

include_HEADERS = \
	Arch.h		\
	Edition.h	\
	ResKind.h	\
	ResName.h	\
	Resolvable.h	\
	Package.h	\
	Selection.h	\
	Message.h	\
	Script.h	\
	Patch.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	Arch.cc		\
	Edition.cc	\
	ResKind.cc	\
	ResName.cc	\
	Resolvable.cc	\
	Package.cc	\
	Selection.cc	\
	Message.cc	\
	Script.cc	\
	Patch.cc

lib@PACKAGE@_la_LDFLAGS =	@LIB_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				detail/lib@PACKAGE@_detail.la

## ##################################################
