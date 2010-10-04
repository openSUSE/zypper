ADD_SUBDIRECTORY(base)

# set the vendor.d paths correctly for the tests
SET(VENDOR_D "${LIBZYPP_SOURCE_DIR}/tests/zypp/data/Vendor/vendors.d")
FILE(MAKE_DIRECTORY "${LIBZYPP_BINARY_DIR}/tests/zypp/data/Vendor")
CONFIGURE_FILE(${LIBZYPP_SOURCE_DIR}/tests/zypp/data/Vendor/zypp2.conf.cmake ${LIBZYPP_BINARY_DIR}/tests/zypp/data/Vendor/zypp2.conf @ONLY)

ADD_TESTS(
  Arch
  Capabilities
  CheckSum
  Date
  Dup
  Digest
  Deltarpm
  Edition
  Fetcher
  FileChecker
  InstanceId
  KeyRing
  Locks
  MediaSetAccess
  PathInfo
  PluginFrame
  PoolQuery
  ProgressData
  PublicKey
  RWPtr
  RepoInfo
  RepoManager
  RepoStatus
  ResKind
  ResStatus
  Selectable
  Target
  Url
  Vendor
  Vendor2
)

