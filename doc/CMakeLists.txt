
SET (my_MAN8
     zypper.8 zypper-log.8)

SET (my_MAN
     ${my_MAN8})

ADD_CUSTOM_TARGET( man ALL
   DEPENDS ${my_MAN}
)

FOREACH (my_MANIDX 1 2 3 4 5 6 7 8)
  INSTALL (FILES
    ${my_MAN${my_MANIDX}}
    DESTINATION ${MANDIR}/man${my_MANIDX}
  )
ENDFOREACH (my_MANIDX)

#
# If a2x is installed, auto update manpage from asciidoc manpage.txt:
#
FIND_PROGRAM(A2X a2x)
FOREACH (my_MANTARGET ${my_MAN})
  IF (A2X)
    ADD_CUSTOM_COMMAND (
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${my_MANTARGET}
      COMMAND ${A2X} -v -f manpage ${my_MANTARGET}.txt
      DEPENDS ${my_MANTARGET}.txt
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  ELSE (A2X)
    ADD_CUSTOM_COMMAND (
      OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${my_MANTARGET}
      DEPENDS ${my_MANTARGET}.txt
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Forgott to fix manpage after changing ascidoc?   Install a2x for automated creation or 'touch ${my_MANTARGET}'"
    )
  ENDIF (A2X)
ENDFOREACH (my_MANTARGET)

