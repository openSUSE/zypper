## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = base parser capability solver source detail

## ##################################################

include_HEADERS = NeedAType.h \
	Arch.h		\
	Capability.h	\
	CapFactory.h	\
	CapSet.h	\
	Dependencies.h	\
	Edition.h	\
	Rel.h		\
	ResObject.h	\
	Resolvable.h	\
	ResTraits.h	\
	Package.h	\
	Selection.h	\
	Message.h	\
	Script.h	\
	Patch.h		\
	Product.h	\
	Changelog.h

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
	Rel.cc		\
	ResObject.cc	\
	Resolvable.cc	\
	ResTraits.cc	\
	Package.cc	\
	Selection.cc	\
	Message.cc	\
	Script.cc	\
	Patch.cc	\
	Product.cc

lib@PACKAGE@_la_LDFLAGS =	@LIB_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        base/lib@PACKAGE@_base.la	\
				detail/lib@PACKAGE@_detail.la	\
				capability/lib@PACKAGE@_capability.la	\
				solver/lib@PACKAGE@_solver.la	\
				parser/lib@PACKAGE@_parser.la	\
				source/lib@PACKAGE@_source.la

## ##################################################
