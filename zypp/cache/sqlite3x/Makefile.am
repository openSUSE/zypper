SUBDIRS =

INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"sqlite3x\"

## ##################################################

sqlite3xincludedir = $(pkgincludedir)/sqlite3x

sqlite3xinclude_HEADERS = sqlite3x.hpp

## ##################################################

noinst_LTLIBRARIES =	lib@PACKAGE@_sqlite3x.la

## ##################################################

lib@PACKAGE@_sqlite3x_la_SOURCES =	sqlite3x_command.cpp sqlite3x_connection.cpp \
	sqlite3x_exception.cpp sqlite3x_reader.cpp sqlite3x_transaction.cpp
	
lib@PACKAGE@_sqlite3x_la_LDFLAGS = -lsqlite3 @LIBZYPP_VERSION_INFO@
