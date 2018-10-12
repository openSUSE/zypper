# Collect all files that may contain translatable texts
FILE( GLOB_RECURSE POT_FILE_DEPENDS RELATIVE ${ZYPPER_SOURCE_DIR} "*.h" "*.cc" )
SET( POT_FILE_DEPENDS ${POT_FILE_DEPENDS} PARENT_SCOPE )

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
  update.h
  download.h
  source-download.h
  subcommand.h
  configtest.h
  solve-commit.h
  PackageArgs.h
  SolverRequester.h
  Summary.h
  callbacks/keyring.h
  callbacks/media.h
  callbacks/rpm.h
  callbacks/repo.h
  callbacks/job.h
  commands/commandhelpformatter.h
  commands/conditions.h
  commands/basecommand.h
  commands/locks.h
  commands/locks/add.h
  commands/locks/clean.h
  commands/locks/list.h
  commands/locks/remove.h
  commands/services.h
  commands/services/common.h
  commands/services/list.h
  commands/services/refresh.h
  commands/services/add.h
  commands/services/modify.h
  commands/services/remove.h
  commands/repos.h
  commands/repos/list.h
  commands/repos/add.h
  commands/repos/clean.h
  commands/repos/remove.h
  commands/repos/rename.h
  commands/repos/modify.h
  commands/repos/refresh.h
  commands/reposerviceoptionsets.h
  commands/ps.h
  commands/needs-rebooting.h
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
  update.cc
  download.cc
  source-download.cc
  subcommand.cc
  configtest.cc
  solve-commit.cc
  PackageArgs.cc
  RequestFeedback.cc
  SolverRequester.cc
  Summary.cc
  callbacks/media.cc
  commands/commandhelpformatter.cc
  commands/conditions.cc
  commands/basecommand.cc
  commands/locks/add.cc
  commands/locks/clean.cc
  commands/locks/list.cc
  commands/locks/remove.cc
  commands/services/common.cc
  commands/services/list.cc
  commands/services/refresh.cc
  commands/services/add.cc
  commands/services/modify.cc
  commands/services/remove.cc
  commands/repos/list.cc
  commands/repos/add.cc
  commands/repos/clean.cc
  commands/repos/remove.cc
  commands/repos/rename.cc
  commands/repos/modify.cc
  commands/repos/refresh.cc
  commands/reposerviceoptionsets.cc
  commands/ps.cc
  commands/needs-rebooting.cc
  ${zypper_HEADERS}
)

SET( zypper_out_HEADERS
  output/Out.h
  output/OutNormal.h
  output/OutXML.h
  output/prompt.h
  output/AliveCursor.h
  output/Utf8.h
)

SET( zypper_out_SRCS
  output/Out.cc
  output/OutNormal.cc
  output/OutXML.cc
  ${zypper_out_HEADERS}
)

SET( zypper_utils_HEADERS
  utils/Augeas.h
  utils/ansi.h
  utils/colors.h
  utils/console.h
  utils/getopt.h
  utils/messages.h
  utils/misc.h
  utils/pager.h
  utils/prompt.h
  utils/richtext.h
  utils/text.h
  utils/XmlFilter.h
  utils/flags/zyppflags.h
  utils/flags/flagtypes.h
  utils/flags/exceptions.h
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
  utils/flags/zyppflags.cc
  utils/flags/flagtypes.cc
  utils/flags/exceptions.cc
  ${zypper_utils_HEADERS}
)

ADD_LIBRARY( zypper_lib STATIC ${zypper_SRCS} ${zypper_out_SRCS} ${zypper_utils_SRCS} )
TARGET_LINK_LIBRARIES( zypper_lib ${ZYPP_LIBRARY} ${READLINE_LIBRARY} -laugeas ${AUGEAS_LIBRARY} -lxml2 )

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
SET_TARGET_PROPERTIES( zypp-refresh PROPERTIES LINK_FLAGS "-pie -Wl,-z,relro,-z,now")
SET_TARGET_PROPERTIES( zypp-refresh PROPERTIES COMPILE_FLAGS "-fwhole-program -fpie -fPIE")
INSTALL(
  TARGETS zypp-refresh
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
