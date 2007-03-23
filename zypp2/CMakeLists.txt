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
  cache/CacheQuery.cc
  cache/CacheStore.cpp
  cache/Utils.cpp
)

SET( zypp2_cache_HEADERS
  cache/CacheCommon.h
  cache/CacheInitializer.h
  cache/CacheStore.h
  cache/CacheQuery.h
  cache/Utils.h
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
)

SET( zypp2_source_cached_HEADERS
  source/cached/CachedSourceImpl.h
)

SET( zypp2_source_sqlite-source_SRCS
  source/sqlite-source/SqliteAccess.cc
  source/sqlite-source/SqliteAtomImpl.cc
  source/sqlite-source/SqliteLanguageImpl.cc
  source/sqlite-source/SqliteMessageImpl.cc
  source/sqlite-source/SqlitePackageImpl.cc
  source/sqlite-source/SqlitePatchImpl.cc
  source/sqlite-source/SqlitePatternImpl.cc
  source/sqlite-source/SqliteProductImpl.cc
  source/sqlite-source/SqliteScriptImpl.cc
  source/sqlite-source/SqliteSourceImpl.cc
  source/sqlite-source/SqliteSources.cc
)

SET( zypp2_source_sqlite-source_HEADERS
  source/sqlite-source/SqliteAccess.h
  source/sqlite-source/SqliteAtomImpl.h
  source/sqlite-source/SqliteLanguageImpl.h
  source/sqlite-source/SqliteMessageImpl.h
  source/sqlite-source/SqlitePackageImpl.h
  source/sqlite-source/SqlitePatchImpl.h
  source/sqlite-source/SqlitePatternImpl.h
  source/sqlite-source/SqliteProductImpl.h
  source/sqlite-source/SqliteScriptImpl.h
  source/sqlite-source/SqliteSourceImpl.h
  source/sqlite-source/SqliteSources.h
  source/sqlite-source/schema.h
)

#INSTALL(  FILES
#  ${zypp2_source_sqlite-source_HEADERS}
#  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypp2/source/sqlite-source
#)


SET( zypp2_lib_SRCS
${zypp2_source_SRCS}
${zypp2_SRCS}
${zypp2_cache_SRCS}
#${zypp2_source_sqlite-source_SRCS}
${zypp2_source_cached_SRCS}
${zypp2_cache_sqlite3x_SRCS}
)

SET( zypp2_lib_HEADERS
${zypp2_source_HEADERS}
${zypp2_HEADERS}
${zypp2_cache_HEADERS}
#${zypp2_source_sqlite-source_HEADERS}
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
