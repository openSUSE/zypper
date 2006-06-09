## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = 					\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"base\"

## ##################################################

baseincludedir = $(pkgincludedir)/base

baseinclude_HEADERS =	\
	Debug.h		\
	Deprecated.h	\
	DefaultFalseBool.h	\
	\
	Algorithm.h	\
	Exception.h	\
	Functional.h	\
	Iterator.h	\
	Logger.h	\
	LogTools.h	\
	LogControl.h	\
	\
	Fd.h		\
	KindOf.h	\
	Gettext.h	\
	GzStream.h	\
	IOStream.h	\
	NonCopyable.h	\
	ProvideNumericId.h	\
	ReferenceCounted.h	\
	SafeBool.h	\
	PtrTypes.h	\
	String.h	\
	Unit.h		\
	Sysconfig.h	\
		\
	Counter.h	\
		\
	ExternalDataSource.h


noinst_LTLIBRARIES =	lib@PACKAGE@_base.la

## ##################################################

lib@PACKAGE@_base_la_SOURCES =	\
	Fd.cc			\
	Gettext.cc		\
	GzStream.cc		\
	IOStream.cc		\
	ReferenceCounted.cc	\
	String.cc		\
	Unit.cc			\
	ExternalDataSource.cc \
	\
	Exception.cc		\
	Sysconfig.cc		\
	LogControl.cc
# NO entries below LogControl.cc
# The static LogControlImpl singleton musr be initialized first!

lib@PACKAGE@_base_la_LIBADD = -lboost_regex

## ##################################################
