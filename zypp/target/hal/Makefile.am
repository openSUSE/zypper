## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS =

INCLUDES = \
        -DDBUS_API_SUBJECT_TO_CHANGE		\
	@HAL_CFLAGS@

#	-I/usr/include/dbus-1.0		
#	-I/usr/lib64/dbus-1.0/include		
#	-I/opt/gnome/include/glib-2.0		
#	-I/opt/gnome/lib64/glib-2.0/include	
#	-I/usr/include/hal

## ##################################################

targethalincludedir = $(pkgincludedir)/target/hal

targethalinclude_HEADERS =	\
	Hal.h


noinst_LTLIBRARIES =	lib@PACKAGE@_target_hal.la

## ##################################################

lib@PACKAGE@_target_hal_la_SOURCES =	\
	Hal.cc

lib@PACKAGE@_target_hal_la_LIBADD =	\
	@HAL_LIBS@

## ##################################################
