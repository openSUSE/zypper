ADD_SUBDIRECTORY(base)

# set the vendor.d paths correctly for the tests
SET(VENDOR_D "${CMAKE_SOURCE_DIR}/tests/zypp/data/Vendor/vendors.d")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/zypp/data/Vendor")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tests/zypp/data/Vendor/zypp2.conf.cmake ${CMAKE_BINARY_DIR}/tests/zypp/data/Vendor/zypp2.conf @ONLY)

ADD_TESTS(
  Arch
  Capabilities
  CheckSum
  Digest
  Deltarpm
  Edition
  Fetcher
  FileChecker
  KeyRing
  Locks
  MediaSetAccess
  PathInfo
  PoolQuery
  ProgressData
  PublicKey
  RWPtr
  RepoInfo
  RepoManager
  RepoStatus
  ResKind
  Target
  Url
  Vendor
  Vendor2
)

#TARGET_LINK_LIBRARIES(Fetcher_test boost_thread-mt)