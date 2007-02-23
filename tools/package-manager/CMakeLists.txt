INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

########### install files ###############

INSTALL(PROGRAMS package-manager package-manager-su DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
INSTALL( FILES package-manager.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" )
INSTALL( FILES  package-manager-icon.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/pixmaps" )
