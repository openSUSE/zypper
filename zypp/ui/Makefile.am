## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

## ##################################################

uiincludedir = $(pkgincludedir)/ui

uiinclude_HEADERS = \
	Status.h		\
	Selectable.h		\
	SelectableTraits.h	\
	ResPoolProxy.h		\
	\
	SelectableImpl.h

noinst_LTLIBRARIES =	lib@PACKAGE@_ui.la

## ##################################################

lib@PACKAGE@_ui_la_SOURCES = \
	Status.cc	\
	Selectable.cc	\
	ResPoolProxy.cc	\
	\
	SelectableImpl.cc

## ##################################################
