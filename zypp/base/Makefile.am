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
	LogControl.cc	\
	Exception.cc	\
	Fd.cc		\
	Gettext.cc	\
	IOStream.cc	\
	ReferenceCounted.cc	\
	String.cc	\
	Unit.cc		\
		\
		\
	ExternalDataSource.cc

lib@PACKAGE@_base_la_LIBADD = -lboost_regex

## ##################################################
