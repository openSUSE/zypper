## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

include_HEADERS =	\
	Debug.h		\
	KindOf.h	\
	Logger.h	\
	Exception.h	\
	NonCopyable.h	\
	PtrTypes.h	\
	ReferenceCounted.h	\
	String.h	\
	StringVal.h		\
				\
				\
	ExternalDataSource.h	\
	stringutil.h


noinst_LTLIBRARIES =	lib@PACKAGE@_base.la

## ##################################################

lib@PACKAGE@_base_la_SOURCES =	\
	Logger.cc	\
	Exception.cc	\
	String.cc	\
	StringVal.cc	\
			\
			\
	ExternalDataSource.cc	\
	stringutil.cc

lib@PACKAGE@_base_la_LIBADD = -lboost_regex

## ##################################################
