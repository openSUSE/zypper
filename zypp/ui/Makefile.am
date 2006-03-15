## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =
AM_CXXFLAGS = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp\"

## ##################################################

uiincludedir = $(pkgincludedir)/ui

uiinclude_HEADERS = 		\
	PatchContents.h		\
	Status.h		\
	Selectable.h		\
	SelFilters.h		\
	SelectableTraits.h	\
				\
	PatchContentsImpl.h	\
	SelectableImpl.h

noinst_LTLIBRARIES =	lib@PACKAGE@_ui.la

## ##################################################

lib@PACKAGE@_ui_la_SOURCES = 	\
	PatchContents.cc	\
	Status.cc		\
	Selectable.cc		\
				\
	PatchContentsImpl.cc	\
	SelectableImpl.cc

## ##################################################
