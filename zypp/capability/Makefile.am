## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

include_HEADERS = \
	CapabilityImpl.h\
	CapTraits.h	\
	Capabilities.h	\
	\
	FileCap.h	\
	HalCap.h	\
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
	NamedCap.cc	\
	NullCap.cc	\
	SplitCap.cc	\
	VersionedCap.cc

## ##################################################
