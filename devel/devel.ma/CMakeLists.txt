## ############################################################

#ADD_SUBDIRECTORY( doc )

## ############################################################

INCLUDE( FindQt3 )
IF ( NOT QT_FOUND )
  MESSAGE( WARNING " qt3 not found" )
ELSE ( NOT QT_FOUND )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )
  INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
  #SET( QT_LIBRARIES
  #"/usr/lib${LIB_SUFFIX}/qt3/lib${LIB_SUFFIX}/libqassistantclient.a;/usr/lib${LIB_SUFFIX}/qt3/lib${LIB_SUFFIX}/libqt-mt.so;-lSM;-lICE;/usr/X11R6/lib${LIB_SUFFIX}/libX11.so;/usr/X11R6/lib${LIB_SUFFIX}/libXext.so;dl;-lpthread"
  #)
ENDIF( NOT QT_FOUND )

SET(THREAD_LIBRARY boost_thread-mt)

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
    #{THREAD_LIBRARY}
   )
ENDFOREACH( loop_var )

## ############################################################
