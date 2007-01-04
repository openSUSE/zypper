INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### install files ###############

INSTALL_FILES( /share/pixmaps FILES  package-manager-icon.png )
INSTALL_FILES(  FILES  package-manager )
INSTALL_FILES( /share/applications FILES  package-manager.desktop )



#original Makefile.am contents follow:

#dist_bin_SCRIPTS = package-manager
#
#icondir = $(datadir)/pixmaps
#icon_DATA = package-manager-icon.png
#
#desktopdir = $(datadir)/applications
#desktop_DATA = package-manager.desktop
#
#EXTRA_DIST = $(icon_DATA) $(desktop_DATA)
