# source:     ${loop_var}_test.cc
# executable: ${loop_var}_test
# test:       ${loop_var}_test
ADD_SUBDIRECTORY(base)

ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK )

#FOREACH( loop_var Vendor1 Vendor2 TranslatedText PoolQuery KeyRing MediaSetAccess ResKind Arch Url Edition Capabilities CheckSum PublicKey PathInfo Digest Fetcher FileChecker RepoManager RepoInfo RepoStatus ProgressData )
FOREACH( loop_var PoolQuery )
   ADD_EXECUTABLE( ${loop_var}_test ${loop_var}_test.cc )
   TARGET_LINK_LIBRARIES( ${loop_var}_test  zypp boost_unit_test_framework)
   ADD_TEST( ${loop_var}_test ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}_test)
ENDFOREACH( loop_var )

ADD_EXECUTABLE(RWPtr RWPtr.cc)
TARGET_LINK_LIBRARIES(RWPtr zypp)

ADD_EXECUTABLE(PtrTypes PtrTypes.cc)
TARGET_LINK_LIBRARIES(PtrTypes zypp  )

#ADD_EXECUTABLE(Signature Signature.cc)
#TARGET_LINK_LIBRARIES(Signature zypp boost_unit_test_framework)

ADD_TEST(PtrTypesTest ${CMAKE_CURRENT_BINARY_DIR}/PtrTypes)

