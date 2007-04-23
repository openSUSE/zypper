ADD_EXECUTABLE( inifile iniparser_test.cc )
TARGET_LINK_LIBRARIES( inifile zypp boost_unit_test_framework )

ADD_TEST(inifile ${CMAKE_CURRENT_BINARY_DIR}/inifile ${CMAKE_CURRENT_SOURCE_DIR}/data )
