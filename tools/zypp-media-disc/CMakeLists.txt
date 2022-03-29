PROJECT( zypp-media-disc C CXX )

set (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_EXTENSIONS OFF)

FIND_PACKAGE(Protobuf REQUIRED)

SET( SOURCES
  main.cc
  discprovider.cc
  discprovider.h
)

add_executable( ${PROJECT_NAME} ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} zypp-media )
target_link_libraries( ${PROJECT_NAME} zypp-core )
target_link_libraries( ${PROJECT_NAME} zypp-protobuf )
target_link_libraries( ${PROJECT_NAME} ${PROTOBUF_LITE_LIBRARIES} )

INSTALL( TARGETS ${PROJECT_NAME} DESTINATION "${ZYPP_LIBEXEC_INSTALL_DIR}" )

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/workers"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/workers"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tools/workers"
)

IF ( UDEV_FOUND )
  TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${UDEV_LIBRARY} )
ELSE ( UDEV_FOUND )
  IF ( HAL_FOUND )
    TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${HAL_LIBRARY} ${HAL_STORAGE_LIBRARY} ${DBUS_LIBRARY} )
  ENDIF ( HAL_FOUND )
ENDIF ( UDEV_FOUND )
