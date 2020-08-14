ADD_SUBDIRECTORY(base)

ADD_TESTS(
  Arch
  Capabilities
  CheckSum
  ContentType
  CpeId
  Date
  DrunkenBishop
  Dup
  Digest
  Deltarpm
  Edition
  ExtendedPool
  FileChecker
  Flags
  GZStream
  InstanceId
  KeyRing
  Locale
  Locks
  PathInfo
  Pathname
  PluginFrame
  PoolQueryCC
  PoolQuery
  ProgressData
  PtrTypes
  PublicKey
  PurgeKernels
  RWPtr
  RepoManager
  RepoStatus
  ResKind
  Resolver
  ResStatus
  RpmPkgSigCheck
  Selectable
  SetRelationMixin
  SetTracker
  StrMatcher
  StringV
  Target
  Url
  UserData
  Vendor
)

IF (ENABLE_ZCHUNK_COMPRESSION)
  ADD_TESTS (
    ZChunk
  )
ENDIF(ENABLE_ZCHUNK_COMPRESSION)

IF( NOT DISABLE_MEDIABACKEND_TESTS )
  ADD_TESTS(
    Fetcher
    MediaSetAccess
    RepoInfo
  )
ENDIF()

