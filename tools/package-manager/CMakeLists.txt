
########### install files ###############

INSTALL(PROGRAMS package-manager package-manager-su DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
INSTALL( FILES package-manager.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" )
