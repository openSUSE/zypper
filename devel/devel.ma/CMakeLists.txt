## ############################################################

INCLUDE( FindQt3 )
IF ( NOT QT_FOUND )
  MESSAGE( WARNING " qt3 not found" )
ELSE ( NOT QT_FOUND )
  ADD_DEFINITIONS( ${QT_DEFINITIONS} )
  INCLUDE_DIRECTORIES( ${QT_INCLUDE_DIR} )
  SET( QT_LIBRARIES
     "/usr/lib64/qt3/lib64/libqassistantclient.a;/usr/lib64/qt3/lib64/libqt-mt.so;-lSM;-lICE;/usr/X11R6/lib64/libX11.so;/usr/X11R6/lib64/libXext.so;dl;-lpthread"
  )
ENDIF( NOT QT_FOUND )

## ############################################################


SET( MA_TEST_SRCS
   MaTest.cc
)
ADD_EXECUTABLE( MA_TEST
   ${MA_TEST_SRCS}
)
TARGET_LINK_LIBRARIES( MA_TEST
   zypp ${QT_LIBRARIES}
)


## ############################################################
