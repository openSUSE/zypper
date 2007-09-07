PROJECT(pdbtozypp)

SET( LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -O2 -Wall -W -fpic -D_REENTRANT" )

FIND_PROGRAM(SWIG_EXECUTABLE
  NAMES swig-1.3 swig
  PATHS ${SWIG_DIR} ${SWIG_DIR}/.. ${SWIG_DIR}/../../bin /usr/bin /usr/local/bin ${CMAKE_INSTALL_PREFIX}/bin
)

IF ( NOT SWIG_EXECUTABLE )
   MESSAGE( FATAL_ERROR " swig not found" )
ELSE ( NOT SWIG_EXECUTABLE )
   MESSAGE( STATUS "SWIG found at ${SWIG_EXECUTABLE}" )
ENDIF ( NOT SWIG_EXECUTABLE ) 

FIND_PACKAGE(Zypp REQUIRED)
IF ( NOT ZYPP_LIBRARY )
   MESSAGE( FATAL_ERROR " zypp not found" )
ELSE ( NOT ZYPP_LIBRARY )
   MESSAGE( STATUS "zypp found" )
ENDIF ( NOT ZYPP_LIBRARY ) 

SET( pdbtozypp_SRCS
      db.cc
      pdbtozypp.cc
      resolvable.cc
   )

SET( pdbtozypp_HEADERS
      db.h
      pdbtozypp.h
      resolvable.h
   )

INSTALL( FILES ${pdbtozypp_HEADERS} DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pdbtozypp" )

ADD_LIBRARY(pdbtozypp SHARED ${pdbtozypp_SRCS})
TARGET_LINK_LIBRARIES(pdbtozypp ${ZYPP_LIBRARY} )
TARGET_LINK_LIBRARIES(pdbtozypp -lmysqlclient )

INSTALL(TARGETS pdbtozypp LIBRARY DESTINATION ${LIB_INSTALL_DIR} )

ADD_SUBDIRECTORY(swig)
