ADD_SUBDIRECTORY( yum )
ADD_SUBDIRECTORY( susetags )

# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  DUdata
  ExtendedMetadata
  MirrorList
  PluginServices
  RepoLicense
  RepoSigcheck
  RepoVariables
)
