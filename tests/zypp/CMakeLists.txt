ADD_SUBDIRECTORY(base)

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

EXEC_PROGRAM( ln ARGS -sf -t "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/data"  )
