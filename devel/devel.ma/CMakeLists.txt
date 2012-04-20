## ############################################################

#ADD_SUBDIRECTORY( doc )

## ############################################################

SET(THREAD_LIBRARY boost_thread)

## ############################################################

ADD_CUSTOM_TARGET( ma_test ALL
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ma_test
   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/ma_test ${CMAKE_CURRENT_BINARY_DIR}/ma_test
)

## ############################################################

FILE( GLOB ALLCC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc" )
STRING( REPLACE ".cc" ";" APLLPROG ${ALLCC} )
FOREACH( loop_var ${APLLPROG} )
   ADD_EXECUTABLE( ${loop_var}
      ${loop_var}.cc
   )
   TARGET_LINK_LIBRARIES( ${loop_var}
    boost_signals
    zypp
    ${THREAD_LIBRARY}
   )
ENDFOREACH( loop_var )

## ############################################################
