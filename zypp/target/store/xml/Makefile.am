## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2 -I..

## ##################################################

targetxmlincludedir = $(pkgincludedir)/target/store/xml

targetxmlinclude_HEADERS =		\
	XMLPatchImpl.h \
	XMLMessageImpl.h \
	XMLScriptImpl.h \
	XMLSelectionImpl.h \
	XMLPatternImpl.h \
	XMLProductImpl.h \
	XMLAtomImpl.h


noinst_LTLIBRARIES =	lib@PACKAGE@_target_xml.la

## ##################################################

lib@PACKAGE@_target_xml_la_SOURCES = \
	XMLPatchImpl.cc \
	XMLMessageImpl.cc \
	XMLScriptImpl.cc \
	XMLSelectionImpl.cc \
	XMLPatternImpl.cc \
	XMLProductImpl.cc \
	XMLAtomImpl.cc
	
## ##################################################
