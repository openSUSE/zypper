#
# Makefile.am for solver/temporary
#

SUBDIRS =

INCLUDES = 						\
	-I$(top_srcdir) -Wall -fno-strict-aliasing	\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"solver_temporary\"

solvertemporaryincludedir = $(pkgincludedir)/solver/temporary

solvertemporaryinclude_HEADERS = \
	utils.h				\
	extract.h			\
	Hash.h				\
	Spec.h				\
	SpecPtr.h			\
	Channel.h			\
	ChannelPtr.h			\
	Subscription.h			\
	ResItem.h			\
	ResItemPtr.h			\
	Package.h			\
	PackagePtr.h			\
	PackageUpdate.h			\
	PackageUpdatePtr.h		\
	XmlNode.h			\
	XmlNodePtr.h			\
	XmlParser.h			\
	Match.h				\
	MatchPtr.h			\
	World.h				\
	WorldPtr.h			\
	UndumpWorld.h			\
	UndumpWorldPtr.h		\
	StoreWorld.h			\
	StoreWorldPtr.h			\
	MultiWorld.h			\
	MultiWorldPtr.h			\
	ServiceWorld.h			\
	ServiceWorldPtr.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver_temporary.la

lib@PACKAGE@_solver_temporary_la_SOURCES =		\
	utils.cc					\
	Spec.cc						\
	Channel.cc					\
	Subscription.cc					\
	ResItem.cc					\
	XmlNode.cc					\
	Package.cc					\
	PackageUpdate.cc				\
	XmlParser.cc					\
	Match.cc					\
	World.cc					\
	UndumpWorld.cc					\
	StoreWorld.cc					\
	MultiWorld.cc					\
	ServiceWorld.cc					\
	extract.cc

