## Process this file with automake to produce Makefile.in
## ##################################################

#INCLUDES

## ##################################################

threadincludedir = $(pkgincludedir)/thread

threadinclude_HEADERS = 	\
	MutexException.h	\
	Mutex.h			\
	MutexLock.h

noinst_LTLIBRARIES =	lib@PACKAGE@_thread.la

## ##################################################

lib@PACKAGE@_thread_la_SOURCES =\
	Mutex.cc

lib@PACKAGE@_thread_la_LDFLAGS = -pthread

## ##################################################
