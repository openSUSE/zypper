INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )


########### install files ###############

INSTALL_FILES( /share/pixmaps FILES  package-manager-icon.png )
INSTALL_FILES(  FILES  package-manager package-manager-su )
INSTALL_FILES( /share/applications FILES  package-manager.desktop )
