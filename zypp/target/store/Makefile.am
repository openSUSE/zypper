## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = xml

## ##################################################

targetstoreincludedir = $(pkgincludedir)/target/store

targetstoreinclude_HEADERS = PersistentStorage.h Backend.h XMLFilesBackend.h serialize.h xml_escape_parser.hpp parser_utils.hpp

noinst_LTLIBRARIES =	lib@PACKAGE@_target_store.la

## ##################################################

lib@PACKAGE@_target_store_la_SOURCES = PersistentStorage.cc Backend.cc XMLFilesBackend.cc serialize.cc xml_escape_parser.cpp md5.cc

lib@PACKAGE@_target_store_la_LIBADD = -lboost_regex -lboost_filesystem \
	xml/lib@PACKAGE@_target_xml.la

## ##################################################