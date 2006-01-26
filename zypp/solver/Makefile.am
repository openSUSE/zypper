## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = detail

## ##################################################

solverincludedir = $(pkgincludedir)/solver

solverinclude_HEADERS = libzypp_solver.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver.la

lib@PACKAGE@_solver_la_LIBADD = detail/lib@PACKAGE@_solver_detail.la

lib@PACKAGE@_solver_la_SOURCES =
