ADD_DEFINITIONS (
  -DPACKAGE=\\\"${PACKAGE}\\\"
  -DLOCALEDIR=\\\"${CMAKE_INSTALL_PREFIX}/share/locale\\\"
)

SET (zypper_HEADERS
  zypper.h
  zypper-main.h
  zypper-command.h
  zypper-getopt.h
  zypper-repos.h
  zypper-misc.h
  zypper-utils.h
  zypper-search.h
  zypper-info.h
  zypper-tabulator.h
  zypper-callbacks.h
  zypper-keyring-callbacks.h
  zypper-media-callbacks.h
  zypper-rpm-callbacks.h
  zypper-repo-callbacks.h
)

SET( zypper_SRCS
  zypper.cc
  zypper-main.cc
  zypper-command.cc
  zypper-getopt.cc
  zypper-repos.cc
  zypper-misc.cc
  zypper-utils.cc
  zypper-search.cc
  zypper-info.cc
  zypper-tabulator.cc
  zypper-callbacks.cc
  ${zypper_HEADERS}
)



ADD_EXECUTABLE( zypper ${zypper_SRCS} )
TARGET_LINK_LIBRARIES( zypper ${ZYPP_LIBRARY} ${READLINE_LIBRARY} )

INSTALL(
  TARGETS zypper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/bin
)

ADD_EXECUTABLE( zypp-checkpatches zypp-checkpatches.cc zypper-updates.cc )
TARGET_LINK_LIBRARIES( zypp-checkpatches ${ZYPP_LIBRARY} )

INSTALL(
  TARGETS zypp-checkpatches
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)

ADD_EXECUTABLE( zypp-checkpatches-wrapper zypp-checkpatches-wrapper.c )
TARGET_LINK_LIBRARIES( zypp-checkpatches-wrapper ${ZYPP_LIBRARY} )

INSTALL(
  TARGETS zypp-checkpatches-wrapper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)


# logrotate config file
INSTALL(
  PROGRAMS installation_sources
  DESTINATION ${INSTALL_PREFIX}/bin
)

# logrotate config file
INSTALL(
  FILES zypper.lr
  DESTINATION ${SYSCONFDIR}/logrotate.d
)

