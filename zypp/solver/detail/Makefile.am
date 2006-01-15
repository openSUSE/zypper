#
# Makefile.am for solver/detail
#

SUBDIRS =

INCLUDES = 						\
	-I$(top_srcdir) -Wall -fno-strict-aliasing	\
	-I/usr/include/libxml2 -I/usr/include/rpm	\
	-DG_LOG_DOMAIN=\"solver_detail\"		\
	-DPHI

solverdetailincludedir = $(pkgincludedir)/solver/detail

solverdetailinclude_HEADERS = \
	ResItemAndDependency.h		\
	ResItemAndDependencyPtr.h	\
	Importance.h			\
	Pending.h			\
	PendingPtr.h			\
	InstallOrder.h			\
	ResolverContext.h		\
	ResolverContextPtr.h		\
	ResolverInfo.h			\
	ResolverInfoPtr.h		\
	ResolverInfoContainer.h		\
	ResolverInfoContainerPtr.h	\
	ResolverInfoChildOf.h		\
	ResolverInfoChildOfPtr.h	\
	ResolverInfoConflictsWith.h	\
	ResolverInfoConflictsWithPtr.h	\
	ResolverInfoDependsOn.h		\
	ResolverInfoDependsOnPtr.h	\
	ResolverInfoMisc.h		\
	ResolverInfoMiscPtr.h		\
	ResolverInfoMissingReq.h	\
	ResolverInfoMissingReqPtr.h	\
	ResolverInfoNeededBy.h		\
	ResolverInfoNeededByPtr.h	\
	ResolverInfoObsoletes.h		\
	ResolverInfoObsoletesPtr.h	\
	QueueItem.h			\
	QueueItemPtr.h			\
	QueueItemBranch.h		\
	QueueItemBranchPtr.h		\
	QueueItemConflict.h		\
	QueueItemConflictPtr.h		\
	QueueItemEstablish.h		\
	QueueItemEstablishPtr.h		\
	QueueItemGroup.h		\
	QueueItemGroupPtr.h		\
	QueueItemInstall.h		\
	QueueItemInstallPtr.h		\
	QueueItemRequire.h		\
	QueueItemRequirePtr.h		\
	QueueItemUninstall.h		\
	QueueItemUninstallPtr.h		\
	ResolverQueue.h			\
	ResolverQueuePtr.h		\
	Resolver.h			\
	ResolverPtr.h

noinst_LTLIBRARIES =	lib@PACKAGE@_solver_detail.la

lib@PACKAGE@_solver_detail_la_SOURCES =			\
	ResItemAndDependency.cc				\
	Importance.cc					\
	Pending.cc					\
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
	ResolverQueue.cc

