## ############################################################

FILE( GLOB ALLCC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" )
STRING( REPLACE ".cc" ";" APLLPROG ${ALLCC} )
FOREACH( loop_var ${APLLPROG} )
  ADD_EXECUTABLE( ${loop_var}
    ${loop_var}.cc
  )
  TARGET_LINK_LIBRARIES( ${loop_var}
    zypp
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
  )
ENDFOREACH( loop_var )

## ############################################################

INSTALL(TARGETS zypp-CheckAccessDeleted DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
INSTALL(TARGETS zypp-NameReqPrv		DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")


SET( ZYPP_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

ADD_SUBDIRECTORY( zypp-rpm )
ADD_SUBDIRECTORY( zypp-media-http )
ADD_SUBDIRECTORY( zypp-media-disc )
ADD_SUBDIRECTORY( zypp-media-nfs )
ADD_SUBDIRECTORY( zypp-media-smb )
ADD_SUBDIRECTORY( zypp-media-dir )
ADD_SUBDIRECTORY( zypp-media-disk )
ADD_SUBDIRECTORY( zypp-media-iso )
ADD_SUBDIRECTORY( zypp-media-tvm )
