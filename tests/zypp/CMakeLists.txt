ADD_SUBDIRECTORY(base)

ADD_TESTS(RWPtr Vendor Vendor2 TranslatedText PoolQuery KeyRing MediaSetAccess ResKind Arch Url Edition Capabilities CheckSum PublicKey PathInfo Digest Fetcher FileChecker RepoManager RepoInfo RepoStatus ProgressData )

EXEC_PROGRAM( ln ARGS -sf "${CMAKE_CURRENT_SOURCE_DIR}/data" "${CMAKE_CURRENT_BINARY_DIR}/data"  )
