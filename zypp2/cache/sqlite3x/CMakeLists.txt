INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(zypp2_sqlite3x_SRCS
sqlite3x_command.cpp
sqlite3x_connection.cpp
sqlite3x_exception.cpp
sqlite3x_reader.cpp
sqlite3x_transaction.cpp
)

ADD_LIBRARY(zypp2_sqlite3x STATIC ${zypp2_sqlite3x_SRCS})

INSTALL_FILES(  FILES  sqlite3x.hpp )



#original Makefile.am contents follow:

#SUBDIRS =
#
#INCLUDES = -DZYPP_BASE_LOGGER_LOGGROUP=\"sqlite3x\"
#
### ##################################################
#
#sqlite3xincludedir = $(pkgincludedir)/sqlite3x
#
#sqlite3xinclude_HEADERS = sqlite3x.hpp
#
### ##################################################
#
#noinst_LTLIBRARIES =	lib@PACKAGE@2_sqlite3x.la
#
### ##################################################
#
#lib@PACKAGE@2_sqlite3x_la_SOURCES =	sqlite3x_command.cpp sqlite3x_connection.cpp \
#	sqlite3x_exception.cpp sqlite3x_reader.cpp sqlite3x_transaction.cpp
#	
#lib@PACKAGE@2_sqlite3x_la_LDFLAGS = -lsqlite3 @LIBZYPP_VERSION_INFO@
