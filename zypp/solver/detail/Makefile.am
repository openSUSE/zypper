#
# Makefile.am for solver/detail
#

SUBDIRS =

INCLUDES = 						\
	-I$(top_srcdir) -Wall -fno-strict-aliasing	\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"solver_detail\"		\
	-DZYPP_BASE_LOGGER_LOGGROUP=\"solver\"		\
	-DPHI

solverdetailincludedir = $(pkgincludedir)/solver/detail

solverdetailinclude_HEADERS = \
	Types.h				\
	Helper.h			\
	Importance.h			\
	Pending.h			\
	InstallOrder.h			\
	ResolverContext.h		\
	ResolverInfo.h			\
	ResolverInfoContainer.h		\
	ResolverInfoChildOf.h		\
	ResolverInfoConflictsWith.h	\
	ResolverInfoDependsOn.h		\
	ResolverInfoMisc.h		\
	ResolverInfoMissingReq.h	\
	ResolverInfoNeededBy.h		\
	ResolverInfoObsoletes.h		\
	QueueItem.h			\
	QueueItemBranch.h		\
	QueueItemConflict.h		\
	QueueItemEstablish.h		\
	QueueItemGroup.h		\
	QueueItemInstall.h		\
	QueueItemRequire.h		\
	QueueItemUninstall.h		\
	ResolverQueue.h			\
	Resolver.h			\
	ResolverUpgrade.cc		\
	ProblemSolutionKeep.h		\
	ProblemSolutionInstall.h	\
	ProblemSolutionUninstall.h	\
	ProblemSolutionIgnore.h		\
	ProblemSolutionUnlock.h		\
	SolutionAction.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver_detail.la

lib@PACKAGE@_solver_detail_la_SOURCES =			\
	Importance.cc					\
	Pending.cc					\
	Helper.cc					\
	InstallOrder.cc					\
	QueueItem.cc					\
	QueueItemBranch.cc				\
	QueueItemConflict.cc				\
	QueueItemEstablish.cc				\
	QueueItemGroup.cc				\
	QueueItemInstall.cc				\
	QueueItemRequire.cc				\
	QueueItemUninstall.cc				\
	Resolver.cc					\
	Resolver_problems.cc				\
	ResolverUpgrade.cc				\
	ResolverInfo.cc					\
	ResolverInfoChildOf.cc				\
	ResolverInfoConflictsWith.cc			\
	ResolverInfoContainer.cc			\
	ResolverInfoDependsOn.cc			\
	ResolverInfoMisc.cc				\
	ResolverInfoMissingReq.cc			\
	ResolverInfoNeededBy.cc				\
	ResolverInfoObsoletes.cc			\
	ResolverContext.cc				\
	ResolverQueue.cc				\
	ProblemSolutionKeep.cc				\
	ProblemSolutionInstall.cc			\
	ProblemSolutionUninstall.cc			\
	ProblemSolutionIgnore.cc			\
	ProblemSolutionUnlock.cc			\
	SolutionAction.cc
