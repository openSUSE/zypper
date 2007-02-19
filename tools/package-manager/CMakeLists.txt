INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### install files ###############

INSTALL_FILES( ${CMAKE_INSTALL_PREFIX}/share/pixmaps FILES  package-manager-icon.png )
INSTALL(PROGRAMS package-manager package-manager-su DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
INSTALL_FILES( ${CMAKE_INSTALL_PREFIX}/share/applications FILES  package-manager.desktop )
