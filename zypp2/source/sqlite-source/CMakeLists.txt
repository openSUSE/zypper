
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

SET(zypp2_source_sqlite_SRCS
SqliteSourceImpl.cc
SqlitePackageImpl.cc
SqliteAtomImpl.cc
SqliteScriptImpl.cc
SqliteMessageImpl.cc
SqliteLanguageImpl.cc
SqlitePatchImpl.cc
SqlitePatternImpl.cc
SqliteProductImpl.cc
SqliteSources.cc
SqliteAccess.cc
schema.h
)

########### next target ###############

ADD_LIBRARY(zypp2_source_sqlite STATIC ${zypp2_source_sqlite_SRCS})


########### install files ###############

#INSTALL_FILES(  FILES  SqliteSourceImpl.h				 	SqlitePackageImpl.h				 	SqliteAtomImpl.h				 	SqliteScriptImpl.h				 	SqliteMessageImpl.h				 	SqliteLanguageImpl.h			 	SqlitePatchImpl.h				 	SqlitePatternImpl.h				 	SqliteProductImpl.h )
#INSTALL_FILES(  FILES  SqliteSources.h				 	SqliteAccess.h )



#original Makefile.am contents follow:

##
## Makefile.am for zmd/backend/dbsource
##
#
#INCLUDES = 					\
#	-I$(top_srcdir) $(ZYPP_CFLAGS) -Wall	\
#	-DG_LOG_DOMAIN=\"sqlite-source\"		\
#	-DZYPP_BASE_LOGGER_LOGGROUP=\"sqlite-source\"
#
#sourceincludedir = $(pkgincludedir)/source
#
#pkginclude_HEADERS =				\
#	SqliteSources.h				\
#	SqliteAccess.h
#
#sourceinclude_HEADERS =				\
#	SqliteSourceImpl.h				\
#	SqlitePackageImpl.h				\
#	SqliteAtomImpl.h				\
#	SqliteScriptImpl.h				\
#	SqliteMessageImpl.h				\
#	SqliteLanguageImpl.h			\
#	SqlitePatchImpl.h				\
#	SqlitePatternImpl.h				\
#	SqliteProductImpl.h
#
#noinst_LTLIBRARIES =	lib@PACKAGE@2_source_sqlite.la
#
#lib@PACKAGE@2_source_sqlite_la_SOURCES =			\
#	SqliteSourceImpl.cc				\
#	SqlitePackageImpl.cc			\
#	SqliteAtomImpl.cc				\
#	SqliteScriptImpl.cc				\
#	SqliteMessageImpl.cc			\
#	SqliteLanguageImpl.cc			\
#	SqlitePatchImpl.cc				\
#	SqlitePatternImpl.cc			\
#	SqliteProductImpl.cc			\
#	SqliteSources.cc				\
#	SqliteAccess.cc \
#	schema.h
#
#lib@PACKAGE@2_source_sqlite_la_LBADD = -lsqlite3
