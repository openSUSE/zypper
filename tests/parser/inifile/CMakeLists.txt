ADD_DEFINITIONS( -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN )

ADD_EXECUTABLE( inifile iniparser_test.cc )
TARGET_LINK_LIBRARIES( inifile zypp boost_unit_test_framework )

ADD_TEST(inifile ${CMAKE_CURRENT_BINARY_DIR}/inifile ${CMAKE_CURRENT_SOURCE_DIR}/data )

ADD_EXECUTABLE( inidict inidict_test.cc )
TARGET_LINK_LIBRARIES( inidict zypp boost_unit_test_framework )

ADD_TEST(inidict ${CMAKE_CURRENT_BINARY_DIR}/inifile ${CMAKE_CURRENT_SOURCE_DIR}/data )
