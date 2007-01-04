ADD_SUBDIRECTORY( tests ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for zypp/target/testsuite
##
#SUBDIRS = tests
#AUTOMAKE_OPTIONS = dejagnu
#PACKAGE = target
#
#EXTRA_DIST = lib/*.exp config/*.exp $(PACKAGE).test/*.exp
#
#clean-local:
#	rm -rf *.log
#	rm -rf *.exp
