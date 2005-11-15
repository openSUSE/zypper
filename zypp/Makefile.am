## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base detail capability parser

## ##################################################

include_HEADERS = \
	Arch.h		\
	Capability.h	\
	CapFactory.h	\
	CapSet.h	\
	Dependencies.h	\
	Edition.h	\
	ResKind.h	\
	Resolvable.h	\
	Package.h	\
	Selection.h	\
	Message.h	\
	Script.h	\
	SolverContext.h	\
	Patch.h		\
	Product.h

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@.la

## ##################################################

lib@PACKAGE@_la_SOURCES = \
	Arch.cc		\
	Capability.cc	\
	CapFactory.cc	\
	CapSet.cc	\
	Dependencies.cc	\
	Edition.cc	\
	ResKind.cc	\
	Resolvable.cc	\
	Package.cc	\
	Selection.cc	\
	Message.cc	\
	Script.cc	\
	SolverContext.cc\
	Patch.cc	\
	Product.cc

lib@PACKAGE@_la_LDFLAGS =	@LIB_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				detail/lib@PACKAGE@_detail.la	\
				capability/lib@PACKAGE@_capability.la
				parser/lib@PACKAGE@_parser.la

## ##################################################
