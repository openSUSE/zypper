## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

baseincludedir = $(pkgincludedir)/base

baseinclude_HEADERS =	\
	Debug.h		\
	Deprecated.h	\
	\
	Algorithm.h	\
	Exception.h	\
	Functional.h	\
	Iterator.h	\
	Logger.h	\
	LogControl.h	\
	\
	Fd.h		\
	KindOf.h	\
	Gettext.h	\
	IOStream.h	\
	NonCopyable.h	\
	PtrTypes.h	\
	ReferenceCounted.h	\
	String.h	\
	Unit.h		\
		\
		\
	ExternalDataSource.h


noinst_LTLIBRARIES =	lib@PACKAGE@_base.la

## ##################################################

lib@PACKAGE@_base_la_SOURCES =	\
	Fd.cc			\
	Gettext.cc		\
	IOStream.cc		\
	ReferenceCounted.cc	\
	String.cc		\
	Unit.cc			\
	ExternalDataSource.cc \
	\
	Exception.cc		\
	LogControl.cc
# NO entries below LogControl.cc
# The static LogControlImpl singleton musr be initialized first!

lib@PACKAGE@_base_la_LIBADD = -lboost_regex

## ##################################################
