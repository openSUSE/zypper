ADD_DEFINITIONS (
  -DPACKAGE=\\\"${PACKAGE}\\\"
  -DLOCALEDIR=\\\"${INSTALL_PREFIX}/share/locale\\\"
)

MESSAGE ( "** Locale dir is ${INSTALL_PREFIX}/share/locale" )

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
  zypper-prompt.h
  zypper-richtext.h
  zypper-tabulator.h
  zypper-locks.h
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
  zypper-prompt.cc
  zypper-richtext.cc
  zypper-locks.cc
  ${zypper_HEADERS}
)

SET( zypper_out_HEADERS
  output/Out.h
  output/OutNormal.h
  output/OutXML.h
  output/prompt.h
  output/AliveCursor.h
)

SET( zypper_out_SRCS
  output/Out.cc
  output/OutNormal.cc
  output/OutXML.cc
  ${zypper_out_HEADERS}
)

ADD_EXECUTABLE( zypper ${zypper_SRCS} ${zypper_out_SRCS} )
TARGET_LINK_LIBRARIES( zypper ${ZYPP_LIBRARY} ${READLINE_LIBRARY} )

INSTALL(
  TARGETS zypper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/bin
)

MESSAGE ( "** ZYpper binary will be installed in ${INSTALL_PREFIX}/bin" )

INSTALL(  FILES
  output/prompt.h
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/zypper
)

INSTALL(  FILES
  output/xmlout.rnc
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/zypper/xml
)

ADD_EXECUTABLE( zypp-checkpatches zypp-checkpatches.cc zypper-updates.cc )
TARGET_LINK_LIBRARIES( zypp-checkpatches ${ZYPP_LIBRARY} )

INSTALL(
  TARGETS zypp-checkpatches
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)

INSTALL(
  FILES bash-completion.sh
  DESTINATION ${SYSCONFDIR}/bash_completion.d
  RENAME zypper.sh
)

ADD_EXECUTABLE( zypp-checkpatches-wrapper zypp-checkpatches-wrapper.c )
TARGET_LINK_LIBRARIES( zypp-checkpatches-wrapper ${ZYPP_LIBRARY} )

INSTALL(
  TARGETS zypp-checkpatches-wrapper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)


# installation_sources script 
INSTALL(
  PROGRAMS installation_sources
  DESTINATION ${INSTALL_PREFIX}/bin
)

# logrotate config file
INSTALL(
  FILES zypper.lr
  DESTINATION ${SYSCONFDIR}/logrotate.d
)

