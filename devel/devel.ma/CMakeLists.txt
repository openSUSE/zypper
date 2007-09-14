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

## ############################################################

ADD_CUSTOM_COMMAND (
   OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/ma_test
   COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/ma_test ${CMAKE_CURRENT_BINARY_DIR}/ma_test
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ma_test
)

ADD_CUSTOM_TARGET( ma_test
   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ma_test
)

## ############################################################

FOREACH( loop_var ScanSource Iorder Xml Ex Main FakePool AOUT Parse Test Basic)
   ADD_EXECUTABLE( ${loop_var}
      ${loop_var}.cc
   )
   TARGET_LINK_LIBRARIES( ${loop_var}
      zypp
   )
ENDFOREACH( loop_var )


FOREACH( loop_var MaTest )
   ADD_EXECUTABLE( ${loop_var}
      ${loop_var}.cc
   )
   TARGET_LINK_LIBRARIES( ${loop_var}
      zypp
      ${QT_LIBRARIES}
   )
ENDFOREACH( loop_var )

## ############################################################
