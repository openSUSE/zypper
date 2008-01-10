FOREACH( loop_var ResKind)
   ADD_EXECUTABLE( ${loop_var}_test ${loop_var}_test.cc )
   TARGET_LINK_LIBRARIES( ${loop_var}_test  zypp boost_unit_test_framework )
   ADD_TEST( ${loop_var}_test ${CMAKE_CURRENT_BINARY_DIR}/${loop_var}_test)
ENDFOREACH( loop_var )


ADD_EXECUTABLE(Arch Arch.cc)
TARGET_LINK_LIBRARIES(Arch zypp boost_unit_test_framework )

ADD_EXECUTABLE(Url Url.cc)
TARGET_LINK_LIBRARIES(Url zypp boost_unit_test_framework )

#ADD_EXECUTABLE(RWPtr RWPtr.cc)
#TARGET_LINK_LIBRARIES(RWPtr zypp boost_unit_test_framework )

ADD_EXECUTABLE(Edition Edition.cc)
TARGET_LINK_LIBRARIES(Edition zypp boost_unit_test_framework )

ADD_EXECUTABLE(Capabilities Capabilities.cc)
TARGET_LINK_LIBRARIES(Capabilities zypp boost_unit_test_framework )

ADD_EXECUTABLE(PtrTypes PtrTypes.cc)
TARGET_LINK_LIBRARIES(PtrTypes zypp  )

ADD_EXECUTABLE(TranslatedText TranslatedText.cc)
TARGET_LINK_LIBRARIES(TranslatedText zypp boost_unit_test_framework)

ADD_EXECUTABLE(CheckSum CheckSum.cc)
TARGET_LINK_LIBRARIES(CheckSum zypp boost_unit_test_framework )

ADD_EXECUTABLE(PublicKey PublicKey.cc)
TARGET_LINK_LIBRARIES(PublicKey zypp boost_unit_test_framework )

ADD_EXECUTABLE(PathInfo PathInfo.cc)
TARGET_LINK_LIBRARIES(PathInfo zypp boost_unit_test_framework )

ADD_EXECUTABLE(Digest Digest.cc)
TARGET_LINK_LIBRARIES(Digest zypp boost_unit_test_framework )

ADD_EXECUTABLE(Signature Signature.cc)
TARGET_LINK_LIBRARIES(Signature zypp boost_unit_test_framework)

ADD_EXECUTABLE(KeyRing KeyRing.cc)
TARGET_LINK_LIBRARIES(KeyRing zypp boost_unit_test_framework )

ADD_EXECUTABLE( MediaSetAccess MediaSetAccessTest.cc )
TARGET_LINK_LIBRARIES( MediaSetAccess zypp boost_unit_test_framework )

ADD_EXECUTABLE( Fetcher Fetcher_test.cc )
TARGET_LINK_LIBRARIES( Fetcher zypp boost_unit_test_framework )

ADD_EXECUTABLE( FileChecker FileChecker_test.cc )
TARGET_LINK_LIBRARIES( FileChecker zypp boost_unit_test_framework )

ADD_EXECUTABLE( RepoManager RepoManager_test.cc )
TARGET_LINK_LIBRARIES( RepoManager zypp boost_unit_test_framework )

ADD_EXECUTABLE( RepoInfo RepoInfo_test.cc )
TARGET_LINK_LIBRARIES( RepoInfo zypp boost_unit_test_framework )

ADD_EXECUTABLE( ProgressData ProgressData_test.cc )
TARGET_LINK_LIBRARIES( ProgressData zypp boost_unit_test_framework )

ADD_TEST(ArchTest ${CMAKE_CURRENT_BINARY_DIR}/Arch)
ADD_TEST(UrlTest ${CMAKE_CURRENT_BINARY_DIR}/Url)
ADD_TEST(Edition ${CMAKE_CURRENT_BINARY_DIR}/Edition)
ADD_TEST(Capabilities ${CMAKE_CURRENT_BINARY_DIR}/Capabilities)
ADD_TEST(PtrTypesTest ${CMAKE_CURRENT_BINARY_DIR}/PtrTypes)
ADD_TEST(TranslatedTextTest ${CMAKE_CURRENT_BINARY_DIR}/TranslatedText)
ADD_TEST(CheckSumTest ${CMAKE_CURRENT_BINARY_DIR}/CheckSum)
ADD_TEST(PublicKeyTest ${CMAKE_CURRENT_BINARY_DIR}/PublicKey)
ADD_TEST(PathInfo ${CMAKE_CURRENT_BINARY_DIR}/PathInfo)
ADD_TEST(Digest ${CMAKE_CURRENT_BINARY_DIR}/Digest)
ADD_TEST(Signature ${CMAKE_CURRENT_BINARY_DIR}/Signature)
ADD_TEST(KeyRingTest ${CMAKE_CURRENT_BINARY_DIR}/KeyRing)
ADD_TEST(MediaSetAccessTest ${CMAKE_CURRENT_BINARY_DIR}/MediaSetAccess ${CMAKE_CURRENT_SOURCE_DIR}/data/mediasetaccess)
ADD_TEST(FileCheckerTest ${CMAKE_CURRENT_BINARY_DIR}/FileChecker ${CMAKE_CURRENT_SOURCE_DIR}/data/FileChecker)
ADD_TEST(RepoManagerTest ${CMAKE_CURRENT_BINARY_DIR}/RepoManager ${CMAKE_CURRENT_SOURCE_DIR}/data/RepoManager)
ADD_TEST(RepoInfoTest ${CMAKE_CURRENT_BINARY_DIR}/RepoInfo ${CMAKE_CURRENT_SOURCE_DIR}/data/RepoInfo)
ADD_TEST(FetcherTest ${CMAKE_CURRENT_BINARY_DIR}/Fetcher)
ADD_TEST(ProgressData_test ${CMAKE_CURRENT_BINARY_DIR}/ProgressData)
