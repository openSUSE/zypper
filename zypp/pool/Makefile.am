## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = 						\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"pool\"

## ##################################################

baseincludedir = $(pkgincludedir)/pool

baseinclude_HEADERS =	\
	PoolTraits.h	\
	PoolImpl.h	\
	\
	PoolStats.h	\
	GetResolvablesToInsDel.h

noinst_LTLIBRARIES =	lib@PACKAGE@_pool.la

## ##################################################

lib@PACKAGE@_pool_la_SOURCES =	\
	PoolImpl.cc	\
	\
	PoolStats.cc	\
	GetResolvablesToInsDel.cc

# lib@PACKAGE@_pool_la_LIBADD =

## ##################################################
