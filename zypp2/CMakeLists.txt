####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

####################################################################
# SOURCES                                                          #
####################################################################

ADD_EXECUTABLE(mkarray ${CMAKE_SOURCE_DIR}/zypp2/cache/schema/mkarray.c )

#FILE(WRITE filename "message to write"... )

ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_BINARY_DIR}/zypp2/schema.h PRE_BUILD COMMAND "${CMAKE_BINARY_DIR}/zypp2/mkarray" ARGS "${CMAKE_SOURCE_DIR}/zypp2/cache/schema/schema.sql" "schemaData" "${CMAKE_BINARY_DIR}/zypp2/schema.h" DEPENDS mkarray)

SET( zypp2_SRCS
  Repository.cc
  RepositoryInfo.cc
  RepositoryManager.cc
  RepositoryFactory.cc
  dummy.cc
)

SET( zypp2_HEADERS
  Repository.h
  RepositoryInfo.h
  RepositoryManager.h
  RepositoryFactory.h
  CommiterIface.h
)

#INSTALL(  FILES ${zypp2_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2
#)

SET( zypp2_cache_SRCS
  ${CMAKE_BINARY_DIR}/zypp2/schema.h
  cache/CacheInitializer.cpp
  cache/CacheTypes.cc
#  cache/QueryFactory.cc
#  cache/CapabilityQuery.cc
  cache/ResolvableQuery.cc
#  cache/CachedResolvableDataProvider.cc
#  cache/sqlite_detail/CapabilityQueryImpl.cc
#  cache/sqlite_detail/QueryFactoryImpl.cc
  cache/CacheStore.cpp
  cache/Utils.cpp
)

SET( zypp2_cache_HEADERS
  cache/CacheCommon.h
  cache/CacheTypes.h
  cache/CacheInitializer.h
  cache/CacheStore.h
  cache/ResolvableQuery.h
#  cache/CapablityQuery.h
#  cache/QueryFactory.h
  cache/Utils.h
#  cache/CachedResolvableDataProvider.h
)

#INSTALL(  FILES
#  ${zypp2_cache_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2/cache
#)

SET( zypp2_cache_sqlite3x_SRCS
  cache/sqlite3x/sqlite3x_command.cpp
  cache/sqlite3x/sqlite3x_connection.cpp
  cache/sqlite3x/sqlite3x_exception.cpp
  cache/sqlite3x/sqlite3x_reader.cpp
  cache/sqlite3x/sqlite3x_transaction.cpp
)

SET( zypp2_cache_sqlite3x_HEADERS
  cache/sqlite3x/sqlite3x.hpp
)

#INSTALL(  FILES
#  ${zypp2_cache_sqlite3x_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2/cache/sqlite3x
#)

SET( zypp2_repository_SRCS
  repository/dummy.cc
  repository/RepositoryImpl.cc
)

SET( zypp2_repository_HEADERS
  repository/dummy.h
  repository/RepositoryImpl.h
)

SET( zypp2_repository_cached_SRCS
  repository/cached/CachedRepositoryImpl.cc
  repository/cached/CachedRepositoryPackageImpl.cc
)

SET( zypp2_repository_cached_HEADERS
  repository/cached/CachedRepositoryImpl.h
  repository/cached/CachedRepositoryPackageImpl.h
)

SET( zypp2_repository_data_SRCS
  repository/memory/DPackageImpl.cc
  repository/memory/DPatternImpl.cc
  repository/memory/DProductImpl.cc
  repository/memory/DSrcPackageImpl.cc
)

SET( zypp2_repository_data_HEADERS
  repository/memory/DPackageImpl.h
  repository/memory/DPatternImpl.h
  repository/memory/DProductImpl.h
  repository/memory/DSrcPackageImpl.h
)

SET( zypp2_parser_susetags_SRCS
  parser/susetags/RepoParser.cc
)

SET( zypp2_parser_susetags_HEADERS
  parser/susetags/RepoParser.h
)


SET( zypp2_parser_yum_SRCS
  parser/yum/YUMParser.cc
)

SET( zypp2_parser_yum_HEADERS
  parser/yum/YUMParser.h
)

#INSTALL(  FILES
#  ${zypp2_repository_sqlite-repository_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2/repository/sqlite-repository
#)


SET( zypp2_lib_SRCS
${zypp2_repository_SRCS}
${zypp2_SRCS}
${zypp2_cache_SRCS}
${zypp2_repository_cached_SRCS}
${zypp2_repository_data_SRCS}
${zypp2_cache_sqlite3x_SRCS}
${zypp2_parser_yum_SRCS}
${zypp2_parser_susetags_SRCS}
)

SET( zypp2_lib_HEADERS
${zypp2_repository_HEADERS}
${zypp2_HEADERS}
${zypp2_cache_HEADERS}
${zypp2_cache_sqlite3x_HEADERS}
${zypp2_parser_yum_HEADERS}
${zypp2_parser_susetags_HEADERS}
)

ADD_LIBRARY(zypp2 SHARED ${zypp2_lib_SRCS})
ADD_DEPENDENCIES(zypp2 schema_header)

# System libraries
TARGET_LINK_LIBRARIES(zypp2 boost_filesystem boost_regex util )
TARGET_LINK_LIBRARIES(zypp2 ${DBUS_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 dbus-glib-1 )
TARGET_LINK_LIBRARIES(zypp2 ${GLIB_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${RPM_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${GETTEXT_LIBRARIES} )
TARGET_LINK_LIBRARIES(zypp2 ${HAL_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${HAL_STORAGE_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${CURL_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${LIBXML_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${SQLITE_LIBRARY} )
TARGET_LINK_LIBRARIES(zypp2 ${ZLIB_LIBRARY} )

# install DB schema
# install XML schemas
FILE( GLOB YUM_SCHEMA_FILES ${CMAKE_SOURCE_DIR}/zypp2/cache/schema/schema.sql )
INSTALL(FILES ${CMAKE_SOURCE_DIR}/zypp2/cache/schema/schema.sql DESTINATION "${CMAKE_INSTALL_PREFIX}/share/zypp/cache" )
