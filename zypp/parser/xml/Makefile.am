## Process this file with automake to produce Makefile.in
## ##################################################

#SUBDIRS =

INCLUDES = -I$(oldincludedir)/libxml2	\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"xml\"

## ##################################################

parserxmlincludedir = $(pkgincludedir)/parser/xml

parserxmlinclude_HEADERS = \
		ParseDef.h	\
		ParseDefConsume.h	\
		ParseDefException.h	\
		ParseDefTraits.h	\
		\
		Reader.h	\
		Node.h		\
		XmlString.h	\
		libxmlfwd.h

noinst_LTLIBRARIES =	lib@PACKAGE@_parser_xml.la


## ##################################################

lib@PACKAGE@_parser_xml_la_SOURCES = \
	ParseDef.cc	\
	ParseDefConsume.cc	\
	ParseDefException.cc	\
	\
	Reader.cc	\
	Node.cc		\
	XmlString.cc	\
	libxmlfwd.cc

lib@PACKAGE@_parser_xml_la_LIBADD =	-lxml2

## ##################################################
