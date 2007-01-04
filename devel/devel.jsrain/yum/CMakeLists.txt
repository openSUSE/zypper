ADD_SUBDIRECTORY( YUMParser.test ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/zypp/parser/yum ${CMAKE_SOURCE_DIR}/zypp ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for packagemanager/testsuite/devel.mir
##
#
#SUBDIRS = YUMParser.test
#
#AUTOMAKE_OPTIONS = dejagnu
#
#PACKAGE = YUMParser
#
#INCLUDES = -I$(top_srcdir)/zypp/parser/yum -I$(top_srcdir)/zypp -I$(oldincludedir)/libxml2 -I$(includedir)
