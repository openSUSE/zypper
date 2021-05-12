
cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW) # if() quoted variables not dereferenced

if(RPM_INCLUDE_DIR AND RPM_LIBRARY)
	# Already in cache, be silent
	set(RPM_FIND_QUIETLY TRUE)
endif(RPM_INCLUDE_DIR AND RPM_LIBRARY)

set(RPM_LIBRARY)
set(RPMIO_LIBRARY)
set(RPM_INCLUDE_DIR)

FIND_PATH(RPM_INCLUDE_DIR rpm/rpmdb.h
	/usr/include
	/usr/local/include
)

set(RPM_SUSPECT_VERSION "RPM_SUSPECT_VERSION-NOTFOUND" )
if ( RPM_INCLUDE_DIR )
	FIND_PATH(RPM_SUSPECT_VERSION rpm/rpm4compat.h
		${RPM_INCLUDE_DIR}
		NO_DEFAULT_PATH
	)
	if ( RPM_SUSPECT_VERSION )
		set(RPM_SUSPECT_VERSION "5.x" )
	else ( RPM_SUSPECT_VERSION )
		set(RPM_SUSPECT_VERSION "4.x" )
	endif ( RPM_SUSPECT_VERSION )
endif ( RPM_INCLUDE_DIR )


FIND_LIBRARY(RPM_LIBRARY NAMES rpm
	PATHS
	/usr/lib
	/usr/local/lib
)

FIND_LIBRARY(RPMIO_LIBRARY NAMES rpmio
	PATHS
	/usr/lib
	/usr/local/lib
)


if(RPM_INCLUDE_DIR AND RPM_LIBRARY AND RPMIO_LIBRARY)
   MESSAGE( STATUS "rpm found: includes in ${RPM_INCLUDE_DIR}, library in ${RPM_LIBRARY}, librpmio in ${RPMIO_LIBRARY} (suspect ${RPM_SUSPECT_VERSION})")

   if ( "${RPM_SUSPECT_VERSION}" STREQUAL "4.x" )
     set( ZYPP_RPM_VERSION_INCL "rpm/rpmlib.h" )
   else()
     set( ZYPP_RPM_VERSION_INCL "rpm/rpmtag.h" )
   endif()

   configure_file( "${CMAKE_CURRENT_LIST_DIR}/printrpmver.c.in"
                   "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/printrpmver.c" @ONLY )

    try_run( RPM_VER_EXITCODE RPM_VER_COMPRESULT
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/printrpmver.c"
      LINK_LIBRARIES ${RPM_LIBRARY}
      CMAKE_FLAGS -DINCLUDE_DIRECTORIES="${RPM_INCLUDE_DIR}"
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      COMPILE_OUTPUT_VARIABLE RPM_PRINTVER_COMPILE_OUTPUT
      RUN_OUTPUT_VARIABLE RPM_LIB_VER
    )

  if ( NOT RPM_VER_COMPRESULT OR NOT RPM_VER_EXITCODE EQUAL 0 )
    message( WARNING "Could not determine rpm version" )
    if ( NOT RPM_VER_COMPRESULT )
      message( WARNING "Compilation failed: ")
      message( WARNING "${RPM_PRINTVER_COMPILE_OUTPUT}")
    endif()
  else ()
    string(REGEX MATCHALL "[^.^-]+" RPM_VER_PARTS "${RPM_LIB_VER}")
    list( LENGTH RPM_VER_PARTS RPM_VER_PARTS_CNT )

    if ( RPM_VER_PARTS_CNT GREATER_EQUAL 3 )

      list( GET RPM_VER_PARTS 0 RPM_LIB_VER_MAJ  )
      list( GET RPM_VER_PARTS 1 RPM_LIB_VER_MIN  )
      list( GET RPM_VER_PARTS 2 RPM_LIB_VER_PATCH )
      message ( STATUS "Detected RPM version is: maj:${RPM_LIB_VER_MAJ}  min:${RPM_LIB_VER_MIN} patch:${RPM_LIB_VER_PATCH}" )
      set(RPM_FOUND TRUE)

    endif()
  endif()


else(RPM_INCLUDE_DIR AND RPM_LIBRARY AND RPMIO_LIBRARY)
   MESSAGE( STATUS "rpm not found")
endif(RPM_INCLUDE_DIR AND RPM_LIBRARY AND RPMIO_LIBRARY)

MARK_AS_ADVANCED(RPM_INCLUDE_DIR RPM_LIBRARY)

cmake_policy(POP)
