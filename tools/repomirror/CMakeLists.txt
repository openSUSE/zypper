PROJECT( repomirror C CXX )

set (CMAKE_CXX_STANDARD 17)
SET (CMAKE_CXX_EXTENSIONS OFF)

SET( SOURCES
  main.cc
)

ADD_DEFINITIONS( -DZYPP_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}" )

message(${Notcurses++_LIBRARIES})

add_executable( ${PROJECT_NAME} ${SOURCES} )
target_link_libraries( ${PROJECT_NAME} zypp ${Notcurses++_LIBRARIES} -lnotcurses-core )
