## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =
AM_CXXFLAGS = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp\"

## ##################################################

uiincludedir = $(pkgincludedir)/ui

uiinclude_HEADERS = 		\
	PatchContents.h		\
	PatternContents.h	\
	Status.h		\
	Selectable.h		\
	SelFilters.h		\
	SelectableTraits.h	\
	UserWantedPackages.h	\
				\
	PatchContentsImpl.h	\
	PatternContentsImpl.h	\
	PatternExpander.h	\
	SelectableImpl.h

noinst_LTLIBRARIES =	lib@PACKAGE@_ui.la

## ##################################################

lib@PACKAGE@_ui_la_SOURCES = 	\
	PatchContents.cc	\
	PatternContents.cc	\
	Status.cc		\
	Selectable.cc		\
	UserWantedPackages.cc	\
				\
	PatchContentsImpl.cc	\
	PatternContentsImpl.cc	\
	PatternExpander.cc	\
	SelectableImpl.cc

## ##################################################
