ADD_SUBDIRECTORY( src ) 
ADD_SUBDIRECTORY( zypp ) 
ADD_SUBDIRECTORY( cache ) 
ADD_SUBDIRECTORY( target ) 
ADD_SUBDIRECTORY( solver ) 
ADD_SUBDIRECTORY( source ) 
ADD_SUBDIRECTORY( media ) 
ADD_SUBDIRECTORY( parser ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for libzypp/testsuite
##
#SUBDIRS = src zypp cache target solver source media parser
