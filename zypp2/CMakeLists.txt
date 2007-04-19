####################################################################
# INCLUDES                                                         #
####################################################################

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

####################################################################
# SOURCES                                                          #
####################################################################


SET( zypp2_SRCS
  Lazy.cc
  dummy.cc
)

SET( zypp2_HEADERS
  CommiterIface.h
  Lazy.h
)

#INSTALL(  FILES ${zypp2_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2
#)

SET( zypp2_cache_SRCS
  cache/CacheInitializer.cpp
  cache/QueryFactory.cc
  cache/CapabilityQuery.cc
  cache/ResolvableQuery.cc
  cache/CachedResolvableDataProvider.cc
  cache/sqlite_detail/CapabilityQueryImpl.cc
  cache/sqlite_detail/QueryFactoryImpl.cc
  cache/CacheStore.cpp
  cache/Utils.cpp
  cache/DatabaseTypes.cc
)

SET( zypp2_cache_HEADERS
  cache/CacheCommon.h
  cache/CacheInitializer.h
  cache/CacheStore.h
  cache/ResolvableQuery.h
  cache/CapablityQuery.h
  cache/QueryFactory.h
  cache/Utils.h
  cache/DatabaseTypes.h
  cache/CachedResolvableDataProvider.h
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

SET( zypp2_source_SRCS
  source/dummy.cc
)

SET( zypp2_source_cached_SRCS
  source/cached/CachedSourceImpl.cc
  source/cached/CachedSourcePackageImpl.cc
)

SET( zypp2_source_cached_HEADERS
  source/cached/CachedSourceImpl.h
  source/cached/CachedSourcePackageImpl.h
)

#INSTALL(  FILES
#  ${zypp2_source_sqlite-source_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2/source/sqlite-source
#)


SET( zypp2_lib_SRCS
${zypp2_source_SRCS}
${zypp2_SRCS}
${zypp2_cache_SRCS}
${zypp2_source_cached_SRCS}
${zypp2_cache_sqlite3x_SRCS}
)

SET( zypp2_lib_HEADERS
${zypp2_source_HEADERS}
${zypp2_HEADERS}
${zypp2_cache_HEADERS}
${zypp2_cache_sqlite3x_HEADERS}
)

ADD_LIBRARY(zypp2 SHARED ${zypp2_lib_SRCS})

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
