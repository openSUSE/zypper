INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET(zypp_solver_detail_SRCS
Importance.cc
Pending.cc
Helper.cc
InstallOrder.cc
QueueItem.cc
QueueItemBranch.cc
QueueItemConflict.cc
QueueItemEstablish.cc
QueueItemGroup.cc
QueueItemInstall.cc
QueueItemRequire.cc
QueueItemUninstall.cc
Resolver.cc
Resolver_problems.cc
ResolverUpgrade.cc
ResolverInfo.cc
ResolverInfoChildOf.cc
ResolverInfoConflictsWith.cc
ResolverInfoContainer.cc
ResolverInfoDependsOn.cc
ResolverInfoMisc.cc
ResolverInfoMissingReq.cc
ResolverInfoNeededBy.cc
ResolverInfoObsoletes.cc
ResolverContext.cc
ResolverQueue.cc
ProblemSolutionKeep.cc
ProblemSolutionInstall.cc
ProblemSolutionUninstall.cc
ProblemSolutionIgnore.cc
ProblemSolutionUnlock.cc
SolutionAction.cc
Testcase.cc
)
ADD_LIBRARY(zypp_solver_detail STATIC ${zypp_solver_detail_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_solver_detail_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"solver_detail\\\")

INSTALL_FILES( FILES
Helper.h
Importance.h
InstallOrder.h
Pending.h
ProblemSolutionIgnore.h
ProblemSolutionInstall.h
ProblemSolutionKeep.h
ProblemSolutionUninstall.h
ProblemSolutionUnlock.h
QueueItemBranch.h
QueueItemConflict.h
QueueItemEstablish.h
QueueItemGroup.h
QueueItem.h
QueueItemInstall.h
QueueItemRequire.h
QueueItemUninstall.h
ResolverContext.h
Resolver.h
ResolverInfoChildOf.h
ResolverInfoConflictsWith.h
ResolverInfoContainer.h
ResolverInfoDependsOn.h
ResolverInfo.h
ResolverInfoMisc.h
ResolverInfoMissingReq.h
ResolverInfoNeededBy.h
ResolverInfoObsoletes.h
ResolverQueue.h
SolutionAction.h
Testcase.h
Types.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/solver/detail
)
