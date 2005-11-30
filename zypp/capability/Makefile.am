## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

include_HEADERS = \
	CapabilityImpl.h\
	Capabilities.h	\
	\
	ConditionalCap.h\
	FileCap.h	\
	NamedCap.h	\
	NullCap.h	\
	OrCap.h		\
	SplitCap.h	\
	VersionedCap.h

noinst_LTLIBRARIES =	lib@PACKAGE@_capability.la

## ##################################################

lib@PACKAGE@_capability_la_SOURCES = \
	CapabilityImpl.cc	\
	\
	FileCap.cc	\
	NamedCap.cc	\
	NullCap.cc	\
	SplitCap.cc	\
	VersionedCap.cc

## ##################################################
