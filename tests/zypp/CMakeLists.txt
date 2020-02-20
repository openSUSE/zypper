ADD_SUBDIRECTORY(base)

# set the vendor.d paths correctly for the tests
SET(VENDOR_D "${LIBZYPP_SOURCE_DIR}/tests/zypp/data/Vendor/vendors.d")
FILE(MAKE_DIRECTORY "${LIBZYPP_BINARY_DIR}/tests/zypp/data/Vendor")
CONFIGURE_FILE(${LIBZYPP_SOURCE_DIR}/tests/zypp/data/Vendor/zypp2.conf.cmake ${LIBZYPP_BINARY_DIR}/tests/zypp/data/Vendor/zypp2.conf @ONLY)

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
  Target
  Url
  UserData
  Vendor
  Vendor2
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

