ADD_SUBDIRECTORY(base)

MESSAGE(STATUS "foo: ${CMAKE_SOURCE_DIR} and ${CMAKE_BINARY_DIR}")

# set the vendor.d paths correctly for the tests
SET(VENDOR_D "${CMAKE_SOURCE_DIR}/tests/zypp/data/Vendor/vendors.d")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests/zypp/data/Vendor")
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/tests/zypp/data/Vendor/zypp2.conf.cmake ${CMAKE_BINARY_DIR}/tests/zypp/data/Vendor/zypp2.conf @ONLY)

ADD_TESTS(
  Arch
  Capabilities
  CheckSum
  Digest
  Edition
  Fetcher
  FileChecker
  KeyRing
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
  TranslatedText
  Url
  Vendor
  Vendor2
)

