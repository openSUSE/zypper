FIND_PROGRAM(ASCIIDOC asciidoctor)
IF (ASCIIDOC)
MESSAGE("asciidoctor found: ${ASCIIDOC}")
SET(ASCDOC_CMD ${ASCIIDOC} -D ${CMAKE_CURRENT_BINARY_DIR} -b manpage -d manpage)
endif()

IF (NOT ASCDOC_CMD)
MESSAGE(FATAL_ERROR "Could not find asciidoctor, please install the package")
ENDIF()

function(ADD_A2X_MANPAGES)
    set(options)
    set(oneValueArgs MAN_INST_PATH)
    set(multiValueArgs MAN_PAGES1 MAN_PAGES2 MAN_PAGES3 MAN_PAGES4 MAN_PAGES5 MAN_PAGES6 MAN_PAGES7 MAN_PAGES8)
    cmake_parse_arguments(A2X "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    set(ALL_MAN_PAGES)

    FOREACH (MAN_IDX RANGE 1 8)
        IF (NOT DEFINED A2X_MAN_PAGES${MAN_IDX})
            CONTINUE()
        ENDIF()

        FOREACH (curr_PAGE ${A2X_MAN_PAGES${MAN_IDX}})
            ADD_CUSTOM_COMMAND (
              OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${curr_PAGE}
              COMMAND ${ASCDOC_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${curr_PAGE}.txt
              DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${curr_PAGE}.txt
              WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
              COMMENT "Building manpage ${curr_PAGE}"
            )

            ADD_CUSTOM_TARGET(
              ${curr_PAGE}_Target ALL
              DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${curr_PAGE}
            )

            INSTALL (
              FILES ${CMAKE_CURRENT_BINARY_DIR}/${curr_PAGE}
              DESTINATION "${A2X_MAN_INST_PATH}/man${MAN_IDX}"
            )

        ENDFOREACH ()
    ENDFOREACH()
endfunction(ADD_A2X_MANPAGES)

SET (my_MAN1
     needs-restarting.1)

SET (my_MAN8
     zypper.8 zypper-log.8 zypp-refresh.8)

ADD_A2X_MANPAGES(
    MAN_INST_PATH ${MANDIR}
    MAN_PAGES1 ${my_MAN1}
    MAN_PAGES8 ${my_MAN8}
)

