ADD_SUBDIRECTORY( src ) 
ADD_SUBDIRECTORY( single.out ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for zypp/solver/testsuite
##
#
#SUBDIRS = src single.out
#AUTOMAKE_OPTIONS = dejagnu
#PACKAGE = solver
#
#EXTRA_DIST = config/*.exp				\
#	    lib/*.exp					\
#	    $(PACKAGE).test/*.exp			\
#	    data.deptestomatic/*-tests/*.xml		\
#	    data.deptestomatic/*-tests/*.solution	\
#	    data.deptestomatic/*-tests/ignore		\
#	    data.deptestomatic/*-tests/xfail		\
#	    data.deptestomatic/*-tests/README
#
#dist-local: clean-local
#
#clean-local:
#	rm -rf data.deptestomatic/*/*.out
#	rm -rf data.deptestomatic/*/*.diff
#	rm -rf data.deptestomatic/*/*.mistake
#	rm -rf single.out/out
