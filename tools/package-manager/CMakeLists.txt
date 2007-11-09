
########### install files ###############

INSTALL(PROGRAMS package-manager package-manager-su DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
INSTALL( FILES package-manager.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications" )
# for D in 16 32 48; do
#   rsvg -w $D -h $D package-manager-icon.svg package-manager-icon.${D}x${D}.png;
# done
INSTALL( FILES  package-manager-icon.svg       DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps" )
INSTALL( FILES  package-manager-icon.16x16.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/16x16/apps" RENAME package-manager-icon.png )
INSTALL( FILES  package-manager-icon.32x32.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/32x32/apps" RENAME package-manager-icon.png )
INSTALL( FILES  package-manager-icon.48x48.png DESTINATION "${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps" RENAME package-manager-icon.png )
