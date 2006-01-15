## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = detail temporary

## ##################################################

include_HEADERS = libzypp_solver.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver.la

lib@PACKAGE@_solver_la_LIBADD = detail/lib@PACKAGE@_solver_detail.la	\
				temporary/lib@PACKAGE@_solver_temporary.la

lib@PACKAGE@_solver_la_SOURCES =
