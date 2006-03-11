## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

capabilityincludedir = $(pkgincludedir)/capability

capabilityinclude_HEADERS = \
	CapabilityImpl.h\
	CapTraits.h	\
	Capabilities.h	\
	\
	FileCap.h	\
	HalCap.h	\
	ModaliasCap.h	\
	NamedCap.h	\
	NullCap.h	\
	SplitCap.h	\
	VersionedCap.h

#	OrCap.h
#	ConditionalCap.h

noinst_LTLIBRARIES =	lib@PACKAGE@_capability.la

## ##################################################

lib@PACKAGE@_capability_la_SOURCES = \
	CapabilityImpl.cc	\
	CapTraits.cc	\
	\
	FileCap.cc	\
	HalCap.cc	\
	ModaliasCap.cc	\
	NamedCap.cc	\
	NullCap.cc	\
	SplitCap.cc	\
	VersionedCap.cc

## ##################################################
