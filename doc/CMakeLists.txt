IF ( ENABLE_BUILD_DOCS )
  ADD_SUBDIRECTORY( autodoc )
ELSE ( ENABLE_BUILD_DOCS )
  ADD_SUBDIRECTORY( autodoc EXCLUDE_FROM_ALL )
ENDIF ( ENABLE_BUILD_DOCS )


SET (libzypp_MAN1
     zypp-CheckAccessDeleted.1 zypp-NameReqPrv.1)

SET (libzypp_MAN5
     locks.5)

SET (libzypp_MAN
     ${libzypp_MAN1} ${libzypp_MAN5})

ADD_CUSTOM_TARGET( man ALL
   DEPENDS ${libzypp_MAN}
)

FOREACH (libzypp_MANIDX 1 2 3 4 5 6 7 8)
  INSTALL (FILES
    ${libzypp_MAN${libzypp_MANIDX}}
    DESTINATION ${MANDIR}/man${libzypp_MANIDX}
  )
ENDFOREACH (libzypp_MANIDX)

#
# If a2x is installed, auto update manpage from asciidoc manpage.txt:
#
FIND_PROGRAM(A2X a2x)
FOREACH (libzypp_MANTARGET ${libzypp_MAN})
  IF (A2X)
    ADD_CUSTOM_COMMAND (
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${libzypp_MANTARGET}
      COMMAND ${A2X} -f manpage ${libzypp_MANTARGET}.txt
      DEPENDS ${libzypp_MANTARGET}.txt
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  ELSE (A2X)
    ADD_CUSTOM_COMMAND (
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${libzypp_MANTARGET}
      DEPENDS ${libzypp_MANTARGET}.txt
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Forgott to fix manpage after changing ascidoc?   Install a2x for automated creation or 'touch ${libzypp_MANTARGET}'"
    )
  ENDIF (A2X)
ENDFOREACH (libzypp_MANTARGET)


