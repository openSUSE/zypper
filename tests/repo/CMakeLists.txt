ADD_SUBDIRECTORY( yum )
ADD_SUBDIRECTORY( susetags )

# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  DUdata
  ExtendedMetadata
  PluginServices
  RepoLicense
  RepoSigcheck
  RepoVariables
)

IF( NOT DISABLE_MEDIABACKEND_TESTS )
ADD_TESTS(
  MirrorList
)
ENDIF()
