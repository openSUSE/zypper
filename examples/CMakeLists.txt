## ############################################################

FILE( GLOB ALLCC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" )
STRING( REPLACE ".cc" ";" APLLPROG ${ALLCC} )
FOREACH( loop_var ${APLLPROG} )
  ADD_EXECUTABLE( ${loop_var}
    ${loop_var}.cc
  )
  TARGET_LINK_LIBRARIES( ${loop_var}
    zypp
  )
ENDFOREACH( loop_var )

## ############################################################
