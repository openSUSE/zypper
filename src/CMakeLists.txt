ADD_DEFINITIONS (
  -DPACKAGE="${PACKAGE}"
  -DLOCALEDIR="${INSTALL_PREFIX}/share/locale"
)

MESSAGE ( "** Locale dir is ${INSTALL_PREFIX}/share/locale" )

INCLUDE_DIRECTORIES(${ZYPPER_SOURCE_DIR}/src)

SET (zypper_HEADERS
  Zypper.h
  main.h
  Command.h
  Config.h
  repos.h
  misc.h
  search.h
  info.h
  Table.h
  locks.h
  update.h
  solve-commit.h
  PackageArgs.h
  SolverRequester.h
  Summary.h
  callbacks/keyring.h
  callbacks/media.h
  callbacks/rpm.h
  callbacks/repo.h
)

SET( zypper_SRCS
  Zypper.cc
  Command.cc
  Config.cc
  repos.cc
  misc.cc
  search.cc
  info.cc
  Table.cc
  locks.cc
  update.cc
  solve-commit.cc
  PackageArgs.cc
  RequestFeedback.cc
  SolverRequester.cc
  Summary.cc
  callbacks/media.cc
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

SET( zypper_utils_HEADERS
  utils/Augeas.h
  utils/colors.h
  utils/console.h
  utils/getopt.h
  utils/messages.h
  utils/misc.h
  utils/pager.h
  utils/prompt.h
  utils/richtext.h
  utils/text.h
)

SET( zypper_utils_SRCS
  utils/Augeas.cc
  utils/colors.cc
  utils/console.cc
  utils/getopt.cc
  utils/messages.cc
  utils/misc.cc
  utils/pager.cc
  utils/prompt.cc
  utils/richtext.cc
  utils/text.cc
  ${zypper_utils_HEADERS}
)

ADD_LIBRARY( zypper_lib ${zypper_SRCS} ${zypper_out_SRCS} ${zypper_utils_SRCS} )
TARGET_LINK_LIBRARIES( zypper_lib ${ZYPP_LIBRARY} ${READLINE_LIBRARY} -laugeas ${AUGEAS_LIBRARY} )

ADD_EXECUTABLE( zypper main.cc )
TARGET_LINK_LIBRARIES( zypper zypper_lib ${ZYPP_LIBRARY} ${READLINE_LIBRARY} -laugeas ${AUGEAS_LIBRARY} -lrt )


INSTALL(
  TARGETS zypper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/bin
)

MESSAGE ( "** ZYpper binary will be installed in ${INSTALL_PREFIX}/bin" )

INSTALL(  FILES
  output/xmlout.rnc
  DESTINATION ${CMAKE_INSTALL_PREFIX}/share/zypper/xml
)

# zypp-refresh utility
ADD_EXECUTABLE( zypp-refresh zypp-refresh.cc )
TARGET_LINK_LIBRARIES( zypp-refresh ${ZYPP_LIBRARY} )
INSTALL(
  TARGETS zypp-refresh
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)

ADD_EXECUTABLE( zypp-refresh-wrapper zypp-refresh-wrapper.c )
TARGET_LINK_LIBRARIES( zypp-refresh-wrapper ${ZYPP_LIBRARY} )
INSTALL(
  TARGETS zypp-refresh-wrapper
  RUNTIME DESTINATION ${INSTALL_PREFIX}/sbin
)

# bash completion script
INSTALL(
  FILES bash-completion.sh
  DESTINATION ${SYSCONFDIR}/bash_completion.d
  RENAME zypper.sh
)


# installation_sources script 
INSTALL(
  PROGRAMS installation_sources
  DESTINATION ${INSTALL_PREFIX}/bin
)

# logrotate config file
INSTALL(
  FILES zypper.lr zypp-refresh.lr
  DESTINATION ${SYSCONFDIR}/logrotate.d
)

# augeas lens
INSTALL(
  FILES utils/zypper.aug
  DESTINATION ${INSTALL_PREFIX}/share/${PACKAGE}
)
