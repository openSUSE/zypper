####################################################################
# SOURCES                                                          #
####################################################################

ADD_DEFINITIONS(-DLOCALEDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/locale\\\" -DTEXTDOMAIN=\\\"zypp\\\" )

ADD_EXECUTABLE(mkarray ${CMAKE_SOURCE_DIR}/zypp/cache/schema/mkarray.c )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
#FILE(WRITE filename "message to write"... )

ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_BINARY_DIR}/zypp/sqlite-schema.h PRE_BUILD COMMAND "${CMAKE_BINARY_DIR}/zypp/mkarray" ARGS "${CMAKE_SOURCE_DIR}/zypp/cache/schema/schema.sql" "schemaData" "${CMAKE_BINARY_DIR}/zypp/sqlite-schema.h" DEPENDS mkarray DEPENDS ${CMAKE_SOURCE_DIR}/zypp/cache/schema/schema.sql )

SET( zypp_SRCS
  ZConfig.cc
  LanguageCode.cc
  Arch.cc
  Atom.cc
  ByteCount.cc
  Capability.cc
  CapFactory.cc
  CapMatch.cc
  CapSet.cc
  CheckSum.cc
  CountryCode.cc
  Date.cc
  Dep.cc
  Dependencies.cc
  Edition.cc
  Language.cc
  Locale.cc
  NVR.cc
  NVRA.cc
  NVRAD.cc
  Range.cc
  Rel.cc
  ResObject.cc
  SystemResObject.cc
  Resolvable.cc
  ResTraits.cc
  ResStatus.cc
  ResStore.cc
  PoolItem.cc
  ResPool.cc
  NameKindProxy.cc
  ResPoolProxy.cc
  ResPoolManager.cc
  Resolver.cc
  ResolverProblem.cc
  ProblemSolution.cc
  Package.cc
  Pathname.cc
  ProgressData.cc
  SrcPackage.cc
  SysContent.cc
  Target.cc
  Selection.cc
  Pattern.cc
  Message.cc
  Script.cc
  Patch.cc
  Product.cc
  Changelog.cc
  ExternalProgram.cc
  PathInfo.cc
  Digest.cc
  Url.cc
  TmpPath.cc
  DiskUsage.cc
  DiskUsageCounter.cc
  VendorAttr.cc
  UpgradeStatistics.cc
  ZYpp.cc
  ZYppCommitPolicy.cc
  ZYppCommitResult.cc
  TranslatedText.cc
  ZYppFactory.cc
  PublicKey.cc
  Signature.cc
  KeyRing.cc
  MediaSetAccess.cc
  ProvideFilePolicy.cc
  OnMediaLocation.cc
  Fetcher.cc
  FileChecker.cc
  Repository.cc
  RepoInfo.cc
  RepoStatus.cc
  RepoManager.cc
  MediaProducts.cc
  Locks.cc
)

SET( zypp_HEADERS
  ZConfig.h
  Arch.h
  Atom.h
  AutoDispose.h
  Bit.h
  ByteCount.h
  Callback.h
  CapAndItem.h
  CapFactory.h
  CapFilters.h
  CapMatch.h
  CapMatchHelper.h
  CapSet.h
  CapSetFwd.h
  Capability.h
  Changelog.h
  CheckSum.h
  CountryCode.h
  Date.h
  Dep.h
  Dependencies.h
  Digest.h
  DiskUsage.h
  DiskUsageCounter.h
  Edition.h
  ExternalProgram.h
  Fetcher.h
  KVMap.h
  KeyRing.h
  Language.h
  LanguageCode.h
  Locale.h
  ManagedFile.h
  MediaSetAccess.h
  Message.h
  NVR.h
  NVRA.h
  NVRAD.h
  NameKindProxy.h
  NeedAType.h
  OnMediaLocation.h
  Package.h
  PackageKeyword.h
  Patch.h
  PathInfo.h
  Pathname.h
  ProgressData.h
  Pattern.h
  PoolItem.h
  ProblemSolution.h
  ProblemTypes.h
  Product.h
  ProvideFilePolicy.h
  PublicKey.h
  Range.h
  Rel.h
  RelCompare.h
  ResFilters.h
  ResObject.h
  ResPool.h
  ResPoolManager.h
  ResPoolProxy.h
  ResStatus.h
  ResStore.h
  ResTraits.h
  Resolvable.h
  Resolver.h
  ResolverProblem.h
  Script.h
  Selection.h
  Signature.h
  SilentCallbacks.h
  SrcPackage.h
  SysContent.h
  SystemResObject.h
  Target.h
  TmpPath.h
  TranslatedText.h
  TriBool.h
  UpgradeStatistics.h
  Url.h
  VendorAttr.h
  ZYpp.h
  ZYppCallbacks.h
  ZYppCommit.h
  ZYppCommitPolicy.h
  ZYppCommitResult.h
  ZYppFactory.h
  FileChecker.h
  Repository.h
  RepoInfo.h
  RepoStatus.h
  RepoManager.h
  MediaProducts.h
  Locks.h
)

INSTALL(  FILES ${zypp_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/zypp" )

SET( zypp_base_SRCS
  base/SerialNumber.cc
  base/Random.cc
  base/Measure.cc
  base/Fd.cc
  base/Gettext.cc
  base/GzStream.cc
  base/IOStream.cc
  base/InputStream.cc
  base/ReferenceCounted.cc
  base/String.cc
  base/Regex.cc
  base/Unit.cc
  base/ExternalDataSource.cc
  base/Exception.cc
  base/UserRequestException.cc
  base/Sysconfig.cc
  base/ProfilingFormater.cc
  base/LogControl.cc
)

SET( zypp_base_HEADERS
  base/SerialNumber.h
  base/Easy.h
  base/Random.h
  base/Algorithm.h
  base/Counter.h
  base/Debug.h
  base/DefaultFalseBool.h
  base/DefaultIntegral.h
  base/Deprecated.h
  base/DtorReset.h
  base/Exception.h
  base/UserRequestException.h
  base/ExternalDataSource.h
  base/Fd.h
  base/Function.h
  base/Functional.h
  base/Gettext.h
  base/GzStream.h
  base/Hash.h
  base/IOStream.h
  base/InputStream.h
  base/Iterator.h
  base/KindOf.h
  base/LogControl.h
  base/LogTools.h
  base/Logger.h
  base/Measure.h
  base/NonCopyable.h
  base/ProfilingFormater.h
  base/ProvideNumericId.h
  base/PtrTypes.h
  base/ReferenceCounted.h
  base/SafeBool.h
  base/String.h
  base/Regex.h
  base/Sysconfig.h
  base/UniqueString.h
  base/Unit.h
  base/WatchFile.h
)

INSTALL(  FILES
  ${zypp_base_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/base
)

SET( zypp_capability_SRCS
  capability/CapTraits.cc
  capability/CapabilityImpl.cc
  capability/FileCap.cc
  capability/HalCap.cc
  capability/ModaliasCap.cc
  capability/FilesystemCap.cc
  capability/NamedCap.cc
  capability/NullCap.cc
  capability/SplitCap.cc
  capability/VersionedCap.cc
)

SET( zypp_capability_HEADERS
  capability/CapTraits.h
  capability/Capabilities.h
  capability/CapabilityImpl.h
  capability/ConditionalCap.h
  capability/FileCap.h
  capability/HalCap.h
  capability/ModaliasCap.h
  capability/FilesystemCap.h
  capability/NamedCap.h
  capability/NullCap.h
  capability/OrCap.h
  capability/SplitCap.h
  capability/VersionedCap.h
)

INSTALL(  FILES
  ${zypp_capability_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/capability
)

SET( zypp_data_SRCS
  data/ResolvableData.cc
  data/RecordId.cc
  data/ResolvableDataConsumer.cc
)

SET( zypp_data_HEADERS
  data/ResolvableData.h
  data/RecordId.h
  data/ResolvableDataConsumer.h
)

INSTALL(  FILES
  ${zypp_data_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/data
)

SET( zypp_detail_SRCS
  detail/AtomImpl.cc
  detail/AtomImplIf.cc
  detail/MessageImpl.cc
  detail/MessageImplIf.cc
  detail/PackageImpl.cc
  detail/PackageImplIf.cc
  detail/PatchImpl.cc
  detail/PatchImplIf.cc
  detail/PatternImpl.cc
  detail/PatternImplIf.cc
  detail/ProductImpl.cc
  detail/ProductImplIf.cc
  detail/ResObjectImplIf.cc
  detail/ResolvableImpl.cc
  detail/ScriptImpl.cc
  detail/ScriptImplIf.cc
  detail/SelectionImpl.cc
  detail/SelectionImplIf.cc
  detail/SrcPackageImpl.cc
  detail/SrcPackageImplIf.cc
)

SET( zypp_detail_HEADERS
  detail/AtomImpl.h
  detail/AtomImplIf.h
  detail/ImplConnect.h
  detail/MessageImpl.h
  detail/MessageImplIf.h
  detail/PackageImpl.h
  detail/PackageImplIf.h
  detail/PatchImpl.h
  detail/PatchImplIf.h
  detail/PatternImpl.h
  detail/PatternImplIf.h
  detail/ProductImpl.h
  detail/ProductImplIf.h
  detail/ResImplTraits.h
  detail/ResObjectFactory.h
  detail/ResObjectImplIf.h
  detail/ResolvableImpl.h
  detail/ScriptImpl.h
  detail/ScriptImplIf.h
  detail/SelectionImpl.h
  detail/SelectionImplIf.h
  detail/SrcPackageImpl.h
  detail/SrcPackageImplIf.h
)

INSTALL(  FILES
  ${zypp_detail_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/detail
)

SET( zypp_media_SRCS
  media/MediaException.cc
  media/MediaAccess.cc
  media/MediaHandler.cc
  media/Mount.cc
  media/MediaNFS.cc
  media/MediaCD.cc
  media/MediaDIR.cc
  media/MediaDISK.cc
  media/MediaSMB.cc
  media/MediaCIFS.cc
  media/ProxyInfo.cc
  media/MediaCurl.cc
  media/MediaISO.cc
  media/MediaManager.cc
  media/MediaUserAuth.cc
)

SET( zypp_media_HEADERS
  media/MediaAccess.h
  media/MediaCD.h
  media/MediaCIFS.h
  media/MediaCurl.h
  media/MediaDIR.h
  media/MediaDISK.h
  media/MediaException.h
  media/MediaHandler.h
  media/MediaISO.h
  media/MediaManager.h
  media/MediaNFS.h
  media/MediaSMB.h
  media/MediaSource.h
  media/MediaUserAuth.h
  media/Mount.h
  media/ProxyInfo.h
)

INSTALL(  FILES
  ${zypp_media_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/media
)

SET( zypp_media_proxyinfo_SRCS
  media/proxyinfo/ProxyInfoSysconfig.cc
)

SET( zypp_media_proxyinfo_HEADERS
  media/proxyinfo/ProxyInfoImpl.h
  media/proxyinfo/ProxyInfoSysconfig.h
  media/proxyinfo/ProxyInfos.h
)

INSTALL(  FILES
  ${zypp_media_proxyinfo_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/media/proxyinfo
)

SET( zypp_parser_SRCS
  parser/ParseException.cc
  parser/TagParser.cc
  parser/IniParser.cc
  parser/IniDict.cc
  parser/LibXMLHelper.cc
  parser/XMLNodeIterator.cc
  parser/RepoFileReader.cc
)

SET( zypp_parser_HEADERS
  parser/ParseException.h
  parser/TagParser.h
  parser/IniParser.h
  parser/IniDict.h
  parser/LibXMLHelper.h
  parser/ParserProgress.h
  parser/XMLNodeIterator.h
  parser/xml_parser_assert.h
  parser/RepoFileReader.h
)

INSTALL(  FILES
  ${zypp_parser_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser
)

SET( zypp_parser_susetags_SRCS
  parser/susetags/RepoIndex.cc
  parser/susetags/ContentFileReader.cc
  parser/susetags/PackagesFileReader.cc
  parser/susetags/PackagesLangFileReader.cc
  parser/susetags/PackagesDuFileReader.cc
  parser/susetags/PatternFileReader.cc
  parser/susetags/RepoParser.cc
)

SET( zypp_parser_susetags_HEADERS
  parser/susetags/RepoIndex.h
  parser/susetags/ContentFileReader.h
  parser/susetags/FileReaderBase.h
  parser/susetags/FileReaderBaseImpl.h
  parser/susetags/PackagesFileReader.h
  parser/susetags/PackagesLangFileReader.h
  parser/susetags/PackagesDuFileReader.h
  parser/susetags/PatternFileReader.h
  parser/susetags/RepoParser.h
)

INSTALL(  FILES
  ${zypp_parser_susetags_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser/susetags
)

SET( zypp_parser_plaindir_SRCS
  parser/plaindir/RepoParser.cc
)

SET( zypp_parser_plaindir_HEADERS
  parser/plaindir/RepoParser.h
)

INSTALL(  FILES
  ${zypp_parser_plaindir_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser/plaindir
)

SET( zypp_parser_xml_SRCS
  parser/xml/Node.cc
  parser/xml/ParseDef.cc
  parser/xml/ParseDefConsume.cc
  parser/xml/ParseDefException.cc
  parser/xml/Reader.cc
  parser/xml/XmlString.cc
  parser/xml/libxmlfwd.cc
)

SET( zypp_parser_xml_HEADERS
  parser/xml/Node.h
  parser/xml/ParseDef.h
  parser/xml/ParseDefConsume.h
  parser/xml/ParseDefException.h
  parser/xml/ParseDefTraits.h
  parser/xml/Reader.h
  parser/xml/XmlString.h
  parser/xml/libxmlfwd.h
)

INSTALL(  FILES
  ${zypp_parser_xml_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser/xml
)

SET( zypp_parser_xmlstore_SRCS
  parser/xmlstore/XMLLanguageParser.cc
  parser/xmlstore/XMLParserData.cc
  parser/xmlstore/XMLPatchParser.cc
  parser/xmlstore/XMLPatternParser.cc
  parser/xmlstore/XMLProductParser.cc
  parser/xmlstore/XMLResObjectParser.cc
  parser/xmlstore/XMLSourceCacheParser.cc
)

SET( zypp_parser_xmlstore_HEADERS
  parser/xmlstore/XMLLanguageParser.h
  parser/xmlstore/XMLParserData.h
  parser/xmlstore/XMLPatchParser.h
  parser/xmlstore/XMLPatternParser.h
  parser/xmlstore/XMLProductParser.h
  parser/xmlstore/XMLResObjectParser.h
  parser/xmlstore/XMLSourceCacheParser.h
  parser/xmlstore/schemanames.h
)

INSTALL(  FILES
  ${zypp_parser_xmlstore_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser/xmlstore
)

SET( zypp_parser_yum_SRCS
  parser/yum/RepoParser.cc
  parser/yum/FileReaderBaseImpl.cc
  parser/yum/RepomdFileReader.cc
  parser/yum/PatchesFileReader.cc
  parser/yum/PrimaryFileReader.cc
  parser/yum/OtherFileReader.cc
  parser/yum/FilelistsFileReader.cc
  parser/yum/PatchFileReader.cc
  parser/yum/PatternFileReader.cc
  parser/yum/ProductFileReader.cc
)

SET( zypp_parser_yum_HEADERS
  parser/yum/RepoParser.h
  parser/yum/RepoParserOpts.h
  parser/yum/FileReaderBase.h
  parser/yum/FileReaderBaseImpl.h
  parser/yum/RepomdFileReader.h
  parser/yum/PatchesFileReader.h
  parser/yum/PrimaryFileReader.h
  parser/yum/OtherFileReader.h
  parser/yum/FilelistsFileReader.h
  parser/yum/PatchFileReader.h
  parser/yum/PatternFileReader.h
  parser/yum/ProductFileReader.h
  parser/yum/schemanames.h
)

INSTALL(  FILES
  ${zypp_parser_yum_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/parser/yum
)

SET( zypp_pool_SRCS
  pool/GetResolvablesToInsDel.cc
  pool/PoolImpl.cc
  pool/PoolStats.cc
)

SET( zypp_pool_HEADERS
  pool/GetResolvablesToInsDel.h
  pool/PoolImpl.h
  pool/PoolStats.h
  pool/PoolTraits.h
)

INSTALL(  FILES
  ${zypp_pool_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/pool
)

SET( zypp_solver_detail_SRCS
  solver/detail/Helper.cc
  solver/detail/Importance.cc
  solver/detail/InstallOrder.cc
  solver/detail/Pending.cc
  solver/detail/ProblemSolutionIgnore.cc
  solver/detail/ProblemSolutionInstall.cc
  solver/detail/ProblemSolutionKeep.cc
  solver/detail/ProblemSolutionUninstall.cc
  solver/detail/ProblemSolutionUnlock.cc
  solver/detail/ProblemSolutionAllBranches.cc
  solver/detail/ProblemSolutionDoubleTimeout.cc
  solver/detail/ProblemSolutionCombi.cc
  solver/detail/QueueItem.cc
  solver/detail/QueueItemBranch.cc
  solver/detail/QueueItemConflict.cc
  solver/detail/QueueItemEstablish.cc
  solver/detail/QueueItemGroup.cc
  solver/detail/QueueItemInstall.cc
  solver/detail/QueueItemRequire.cc
  solver/detail/QueueItemUninstall.cc
  solver/detail/Resolver.cc
  solver/detail/ResolverContext.cc
  solver/detail/ResolverInfo.cc
  solver/detail/ResolverInfoChildOf.cc
  solver/detail/ResolverInfoConflictsWith.cc
  solver/detail/ResolverInfoContainer.cc
  solver/detail/ResolverInfoDependsOn.cc
  solver/detail/ResolverInfoMisc.cc
  solver/detail/ResolverInfoMissingReq.cc
  solver/detail/ResolverInfoNeededBy.cc
  solver/detail/ResolverInfoObsoletes.cc
  solver/detail/ResolverQueue.cc
  solver/detail/ResolverUpgrade.cc
  solver/detail/Resolver_problems.cc
  solver/detail/SolutionAction.cc
  solver/detail/Testcase.cc
  solver/detail/ContextPool.cc
)

SET( zypp_solver_detail_HEADERS
  solver/detail/Helper.h
  solver/detail/Importance.h
  solver/detail/InstallOrder.h
  solver/detail/Pending.h
  solver/detail/ProblemSolutionIgnore.h
  solver/detail/ProblemSolutionInstall.h
  solver/detail/ProblemSolutionKeep.h
  solver/detail/ProblemSolutionUninstall.h
  solver/detail/ProblemSolutionUnlock.h
  solver/detail/ProblemSolutionAllBranches.h
  solver/detail/ProblemSolutionDoubleTimeout.h
  solver/detail/ProblemSolutionCombi.h
  solver/detail/QueueItem.h
  solver/detail/QueueItemBranch.h
  solver/detail/QueueItemConflict.h
  solver/detail/QueueItemEstablish.h
  solver/detail/QueueItemGroup.h
  solver/detail/QueueItemInstall.h
  solver/detail/QueueItemRequire.h
  solver/detail/QueueItemUninstall.h
  solver/detail/Resolver.h
  solver/detail/ResolverContext.h
  solver/detail/ResolverInfo.h
  solver/detail/ResolverInfoChildOf.h
  solver/detail/ResolverInfoConflictsWith.h
  solver/detail/ResolverInfoContainer.h
  solver/detail/ResolverInfoDependsOn.h
  solver/detail/ResolverInfoMisc.h
  solver/detail/ResolverInfoMissingReq.h
  solver/detail/ResolverInfoNeededBy.h
  solver/detail/ResolverInfoObsoletes.h
  solver/detail/ResolverQueue.h
  solver/detail/SolutionAction.h
  solver/detail/Testcase.h
  solver/detail/Types.h
  solver/detail/ContextPool.h
)

INSTALL(  FILES
  ${zypp_solver_detail_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/solver/detail
)

SET( zypp_sat_SRCS
  sat/detail/PoolImpl.cc
  sat/Pool.cc
  sat/Repo.cc
  sat/Solvable.cc
  sat/IdStr.cc
  sat/SATResolver.cc
)

SET( zypp_sat_HEADERS
  sat/detail/PoolMember.h
  sat/detail/PoolImpl.h
  sat/Pool.h
  sat/Repo.h
  sat/Solvable.h
  sat/IdStr.h
  sat/SATResolver.h
)

INSTALL(  FILES
  ${zypp_sat_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/sat
)

SET( zypp_target_SRCS
  target/CommitLog.cc
  target/CommitPackageCache.cc
  target/CommitPackageCacheImpl.cc
  target/CommitPackageCacheReadAhead.cc
  target/TargetCallbackReceiver.cc
  target/TargetException.cc
  target/TargetImpl.cc
)

SET( zypp_target_HEADERS
  target/CommitLog.h
  target/CommitPackageCache.h
  target/CommitPackageCacheImpl.h
  target/CommitPackageCacheReadAhead.h
  target/TargetCallbackReceiver.h
  target/TargetException.h
  target/TargetImpl.h
)

INSTALL(  FILES
  ${zypp_target_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target
)

SET( zypp_target_hal_SRCS
  target/hal/Hal.cc
  target/hal/HalContext.cc
)

SET( zypp_target_hal_HEADERS
  target/hal/Hal.h
  target/hal/HalContext.h
  target/hal/HalException.h
)

INSTALL(  FILES
  ${zypp_target_hal_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target/hal
)

SET( zypp_target_modalias_SRCS
  target/modalias/Modalias.cc
)

SET( zypp_target_modalias_HEADERS
  target/modalias/Modalias.h
)

INSTALL(  FILES
  ${zypp_target_modalias_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target/modalias
)

SET( zypp_target_rpm_SRCS
  target/rpm/BinHeader.cc
  target/rpm/RpmCallbacks.cc
  target/rpm/RpmDb.cc
  target/rpm/RpmException.cc
  target/rpm/RpmHeader.cc
  target/rpm/RpmPackageImpl.cc
  target/rpm/librpmDb.cc
  target/rpm/librpmDb.cv3.cc
)

SET( zypp_target_rpm_HEADERS
  target/rpm/BinHeader.h
  target/rpm/RpmCallbacks.h
  target/rpm/RpmDb.h
  target/rpm/RpmException.h
  target/rpm/RpmHeader.h
  target/rpm/RpmPackageImpl.h
  target/rpm/librpm.h
  target/rpm/librpmDb.h
)

INSTALL(  FILES
  ${zypp_target_rpm_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target/rpm
)

SET( zypp_target_store_SRCS
  target/store/Backend.cc
  target/store/PersistentStorage.cc
  target/store/XMLFilesBackend.cc
  target/store/serialize.cc
  target/store/xml_escape_parser.cpp
)

SET( zypp_target_store_HEADERS
  target/store/Backend.h
  target/store/PersistentStorage.h
  target/store/XMLFilesBackend.h
  target/store/parser_utils.hpp
  target/store/serialize.h
  target/store/xml_escape_parser.hpp
)

INSTALL(  FILES
  ${zypp_target_store_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target/store
)

SET( zypp_target_store_xml_SRCS
  target/store/xml/XMLAtomImpl.cc
  target/store/xml/XMLMessageImpl.cc
  target/store/xml/XMLPatchImpl.cc
  target/store/xml/XMLPatternImpl.cc
  target/store/xml/XMLProductImpl.cc
  target/store/xml/XMLScriptImpl.cc
  target/store/xml/XMLSelectionImpl.cc
)

SET( zypp_target_store_xml_HEADERS
  target/store/xml/XMLAtomImpl.h
  target/store/xml/XMLMessageImpl.h
  target/store/xml/XMLPatchImpl.h
  target/store/xml/XMLPatternImpl.h
  target/store/xml/XMLProductImpl.h
  target/store/xml/XMLScriptImpl.h
  target/store/xml/XMLSelectionImpl.h
)

INSTALL(  FILES
  ${zypp_target_store_xml_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/target/store/xml
)

SET( zypp_thread_SRCS
  thread/Mutex.cc
)

SET( zypp_thread_HEADERS
  thread/Mutex.h
  thread/MutexException.h
  thread/MutexLock.h
  thread/Once.h
)

INSTALL(  FILES
  ${zypp_thread_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/thread
)

SET( zypp_ui_SRCS
  ui/PatchContents.cc
  ui/PatchContentsImpl.cc
  ui/PatternContents.cc
  ui/PatternContentsImpl.cc
  ui/PatternExpander.cc
  ui/Selectable.cc
  ui/SelectableImpl.cc
  ui/Status.cc
  ui/UserWantedPackages.cc
)

SET( zypp_ui_HEADERS
  ui/PatchContents.h
  ui/PatchContentsImpl.h
  ui/PatternContents.h
  ui/PatternContentsImpl.h
  ui/PatternExpander.h
  ui/SelFilters.h
  ui/Selectable.h
  ui/SelectableImpl.h
  ui/SelectableTraits.h
  ui/Status.h
  ui/UserWantedPackages.h
)

INSTALL(  FILES
  ${zypp_ui_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/ui
)

SET( zypp_url_SRCS
  url/UrlUtils.cc
  url/UrlBase.cc
)

SET( zypp_url_HEADERS
  url/UrlBase.h
  url/UrlException.h
  url/UrlUtils.h
)

INSTALL(  FILES
  ${zypp_url_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/url
)

SET( zypp_zypp_detail_SRCS
  zypp_detail/ZYppImpl.cc
)

SET( zypp_zypp_detail_HEADERS
  zypp_detail/ZYppImpl.h
  zypp_detail/ZYppReadOnlyHack.h
)

INSTALL(  FILES
  ${zypp_zypp_detail_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/zypp_detail
)

SET( zypp_cache_SRCS
  ${CMAKE_BINARY_DIR}/zypp/sqlite-schema.h
  cache/CacheInitializer.cc
  cache/CacheException.cc
  cache/Attribute.cc
  cache/CacheTypes.cc
  cache/ResolvableQuery.cc
  cache/CacheStore.cc
  cache/CacheFSCK.cc
  cache/Utils.cc
)

SET( zypp_cache_HEADERS
  cache/Attribute.h
  cache/CacheAttributes.h
  cache/CacheTypes.h
  cache/CacheInitializer.h
  cache/CacheException.h
  cache/CacheStore.h
  cache/CacheFSCK.h
  cache/ResolvableQuery.h
  cache/Utils.h
)

INSTALL(  FILES
  ${zypp_cache_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/cache
)

SET( zypp_cache_sqlite3x_SRCS
  cache/sqlite3x/sqlite3x_command.cpp
  cache/sqlite3x/sqlite3x_connection.cpp
  cache/sqlite3x/sqlite3x_exception.cpp
  cache/sqlite3x/sqlite3x_reader.cpp
  cache/sqlite3x/sqlite3x_transaction.cpp
)

SET( zypp_cache_sqlite3x_HEADERS
  cache/sqlite3x/sqlite3x.hpp
)

#INSTALL(  FILES
#  ${zypp_cache_sqlite3x_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/cache/sqlite3x
#)

SET( zypp_repo_SRCS
  repo/dummy.cc
  repo/RepositoryImpl.cc
  repo/RepoException.cc
  repo/RepoType.cc
  repo/PackageProvider.cc
  repo/SrcPackageProvider.cc
  repo/ScriptProvider.cc
  repo/RepoProvideFile.cc
  repo/DeltaCandidates.cc
  repo/Applydeltarpm.cc
  repo/PackageDelta.cc
  repo/SUSEMediaVerifier.cc
  repo/MediaInfoDownloader.cc
  repo/Downloader.cc
  repo/RepoVariables.cc
)

SET( zypp_repo_HEADERS
  repo/RepositoryImpl.h
  repo/RepoException.h
  repo/RepoType.h
  repo/PackageProvider.h
  repo/SrcPackageProvider.h
  repo/ScriptProvider.h
  repo/RepoProvideFile.h
  repo/DeltaCandidates.h
  repo/Applydeltarpm.h
  repo/PackageDelta.h
  repo/SUSEMediaVerifier.h
  repo/MediaInfoDownloader.h
  repo/Downloader.h
  repo/RepoVariables.h
)

INSTALL( FILES
  ${zypp_repo_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp/repo
)

SET( zypp_repo_cached_SRCS
  repo/cached/RepoImpl.cc
  repo/cached/PackageImpl.cc
  repo/cached/SrcPackageImpl.cc
  repo/cached/PatchImpl.cc
  repo/cached/PatternImpl.cc
  repo/cached/ProductImpl.cc
  repo/cached/MessageImpl.cc
  repo/cached/ScriptImpl.cc
  repo/cached/AtomImpl.cc
)

SET( zypp_repo_cached_HEADERS
  repo/cached/RepoImpl.h
  repo/cached/PackageImpl.h
  repo/cached/SrcPackageImpl.h
  repo/cached/PatchImpl.h
  repo/cached/PatternImpl.h
  repo/cached/ProductImpl.h
  repo/cached/MessageImpl.h
  repo/cached/ScriptImpl.h
  repo/cached/AtomImpl.h
)

SET( zypp_repo_yum_SRCS
  repo/yum/Downloader.cc
  repo/yum/ResourceType.cc
)

SET( zypp_repo_yum_HEADERS
  repo/yum/Downloader.h
  repo/yum/ResourceType.h
)

SET( zypp_repo_susetags_SRCS
  repo/susetags/Downloader.cc
)

SET( zypp_repo_susetags_HEADERS
  repo/susetags/Downloader.h
)


SET( zypp_repo_data_SRCS
  repo/memory/ScriptImpl.cc
  repo/memory/MessageImpl.cc
  repo/memory/PackageImpl.cc
  repo/memory/PatternImpl.cc
  repo/memory/ProductImpl.cc
  repo/memory/PatchImpl.cc
  repo/memory/RepoImpl.cc
  repo/memory/AtomImpl.cc
  repo/memory/SrcPackageImpl.cc
)

SET( zypp_repo_data_HEADERS
  repo/memory/PackageImpl.h
  repo/memory/PatternImpl.h
  repo/memory/ProductImpl.h
  repo/memory/PatchImpl.h
  repo/memory/RepoImpl.h
  repo/memory/AtomImpl.h
  repo/memory/SrcPackageImpl.h
)

SET( zypp_lib_SRCS
${zypp_detail_SRCS}
${zypp_capability_SRCS}
${zypp_cache_SRCS}
${zypp_cache_sqlite3x_SRCS}
${zypp_pool_SRCS}
${zypp_parser_susetags_SRCS}
${zypp_parser_xmlstore_SRCS}
${zypp_parser_xml_SRCS}
${zypp_parser_yum_SRCS}
${zypp_parser_plaindir_SRCS}
${zypp_parser_SRCS}
${zypp_data_SRCS}
${zypp_media_proxyinfo_SRCS}
${zypp_media_SRCS}
${zypp_url_SRCS}
${zypp_repo_SRCS}
${zypp_repo_cached_SRCS}
${zypp_repo_yum_SRCS}
${zypp_repo_susetags_SRCS}
${zypp_repo_data_SRCS}
${zypp_target_store_xml_SRCS}
${zypp_target_store_SRCS}
${zypp_target_rpm_SRCS}
${zypp_target_hal_SRCS}
${zypp_target_modalias_SRCS}
${zypp_target_SRCS}
${zypp_solver_detail_SRCS}
${zypp_sat_SRCS}
${zypp_ui_SRCS}
${zypp_zypp_detail_SRCS}
${zypp_thread_SRCS}
${zypp_SRCS}
${zypp_base_SRCS}
)

SET( zypp_lib_HEADERS
${zypp_target_store_xml_HEADERS}
${zypp_target_rpm_HEADERS}
${zypp_parser_tagfile_HEADERS}
${zypp_parser_susetags_HEADERS}
${zypp_parser_yum_HEADERS}
${zypp_parser_plaindir_HEADERS}
${zypp_parser_xml_HEADERS}
${zypp_parser_HEADERS}
${zypp_capability_HEADERS}
${zypp_cache_HEADERS}
${zypp_cache_sqlite3x_HEADERS}
${zypp_ui_HEADERS}
${zypp_parser_xmlstore_HEADERS}
${zypp_data_HEADERS}
${zypp_media_HEADERS}
${zypp_target_hal_HEADERS}
${zypp_media_proxyinfo_HEADERS}
${zypp_base_HEADERS}
${zypp_solver_detail_HEADERS}
${zypp_sat_HEADERS}
${zypp_target_store_HEADERS}
${zypp_url_HEADERS}
${zypp_HEADERS}
${zypp_zypp_detail_HEADERS}
${zypp_thread_HEADERS}
${zypp_repo_HEADERS}
${zypp_source_susetags_HEADERS}
${zypp_target_modalias_HEADERS}
${zypp_target_HEADERS}
${zypp_detail_HEADERS}
${zypp_pool_HEADERS}
)

#
# Macro to set the log group for a list of files
#
MACRO( SET_LOGGROUP _group _files  )
  SET_SOURCE_FILES_PROPERTIES( ${_files} COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\"${_group}\\" )
  FOREACH (_currentFile ${ARGN})
#MESSAGE( STATUS "setting loggroup to \"${_group}\" for ${_currentFile}" )
    SET_SOURCE_FILES_PROPERTIES( ${_currentFile} COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\"${_group}\\" )
  ENDFOREACH (_currentFile ${ARGN})
ENDMACRO( SET_LOGGROUP )

# Default loggroup for all files
SET_LOGGROUP( "zypp" ${zypp_lib_SRCS} )

# override some defaults
SET_LOGGROUP( "xmlstore" ${zypp_target_store_xml_SRCS} )
SET_LOGGROUP( "satsolver" ${zypp_sat_SRCS} )
#SET_LOGGROUP( "group" ${zypp_target_rpm_SRCS} )
#SET_LOGGROUP( "group" ${zypp_parser_yum2_SRCS} )
#SET_LOGGROUP( "group" ${zypp_capability_SRCS} )
#SET_LOGGROUP( "group" ${zypp_ui_SRCS} )
#SET_LOGGROUP( "group" ${zypp_parser_xmlstore_SRCS} )
#SET_LOGGROUP( "group" ${zypp_data_SRCS} )
#SET_LOGGROUP( "group" ${zypp_media_SRCS} )
#SET_LOGGROUP( "group" ${zypp_target_hal_SRCS} )
#SET_LOGGROUP( "group" ${zypp_parser_xml_SRCS} )
#SET_LOGGROUP( "group" ${zypp_media_proxyinfo_SRCS} )
#SET_LOGGROUP( "group" ${zypp_source_yum_SRCS} )
#SET_LOGGROUP( "group" ${zypp_base_SRCS} )
#SET_LOGGROUP( "group" ${zypp_solver_detail_SRCS} )
#SET_LOGGROUP( "group" ${zypp_sat_SRCS} )
#SET_LOGGROUP( "group" ${zypp_target_store_SRCS} )
#SET_LOGGROUP( "group" ${zypp_url_SRCS} )
#SET_LOGGROUP( "group" ${zypp_source_SRCS} )
#SET_LOGGROUP( "group" ${zypp_parser_SRCS} )
#SET_LOGGROUP( "group" ${zypp_SRCS} )
#SET_LOGGROUP( "group" ${zypp_zypp_detail_SRCS} )
#SET_LOGGROUP( "group" ${zypp_thread_SRCS} )
#SET_LOGGROUP( "group" ${zypp_source_susetags_SRCS} )
#SET_LOGGROUP( "group" ${zypp_target_modalias_SRCS} )
#SET_LOGGROUP( "group" ${zypp_target_SRCS} )
#SET_LOGGROUP( "group" ${zypp_parser_yum_SRCS} )
#SET_LOGGROUP( "group" ${zypp_detail_SRCS} )
#SET_LOGGROUP( "group" ${zypp_pool_SRCS} )


ADD_LIBRARY(zypp SHARED ${zypp_lib_SRCS})
SET_TARGET_PROPERTIES( zypp PROPERTIES VERSION "${LIBZYPP_VERSION_INFO}" )
SET_TARGET_PROPERTIES( zypp PROPERTIES SOVERSION "${LIBZYPP_SOVERSION_INFO}" )
ADD_DEPENDENCIES(zypp schema_header)
# System libraries
TARGET_LINK_LIBRARIES(zypp boost_filesystem util )
TARGET_LINK_LIBRARIES(zypp ${DBUS_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp dbus-glib-1 )
TARGET_LINK_LIBRARIES(zypp ${GLIB_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${RPM_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${GETTEXT_LIBRARIES} )
TARGET_LINK_LIBRARIES(zypp ${HAL_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${HAL_STORAGE_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${CURL_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${LIBXML_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${SQLITE_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${ZLIB_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp ${SATSOLVER_LIBRARY} )

INSTALL(TARGETS zypp LIBRARY DESTINATION ${LIB_INSTALL_DIR} )

# install XML schemas
FILE( GLOB YUM_SCHEMA_FILES ${CMAKE_SOURCE_DIR}/zypp/parser/yum/schema/*.rng )
INSTALL(FILES ${YUM_SCHEMA_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/zypp/schema/yum" )

FILE( GLOB XMLSTORE_SCHEMA_FILES ${CMAKE_SOURCE_DIR}/zypp/parser/xmlstore/schema/*.rng )
INSTALL(FILES ${XMLSTORE_SCHEMA_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/zypp/schema/xmlstore" )
