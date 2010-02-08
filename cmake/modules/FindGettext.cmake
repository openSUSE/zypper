# - Find GNU gettext tools
# This module looks for the GNU gettext tools. This module defines the
# following values:
#  GETTEXT_MSGMERGE_EXECUTABLE: the full path to the msgmerge tool.
#  GETTEXT_MSGFMT_EXECUTABLE: the full path to the msgfmt tool.
#  GETTEXT_FOUND: True if gettext has been found.
#
# Additionally it provides the following macros:
# GETTEXT_CREATE_TRANSLATIONS ( _moBasename [ALL] file1 ... fileN )
#    This will create a target "translations" which will convert the
#    given input po files into the binary output mo file. If the
#    ALL option is used, the translations will also be created when
#    building the default target.

FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

#
# Macro to be called if .po files are shipped as tar ball.
#
# _translation_set_basename: Serves two purposes; a) the stem of the
# default tarball %{_translation_set_basename}-po.tar.bz2 unless its over
# riden by -DUSE_TRANSLATION_SET; b) the basename of the .gmo files.
#
# We expect a po-file tarball to unpack the .po file to the current
# directory!
#
MACRO( GETTEXT_CREATE_TARBALL_TRANSLATIONS _translation_set_basename )

        IF( NOT USE_TRANSLATION_SET )
                SET( USE_TRANSLATION_SET ${_translation_set_basename} )
        ENDIF( NOT USE_TRANSLATION_SET )

        SET( TRANSLATION_SET "${USE_TRANSLATION_SET}-po.tar.bz2" )
        MESSAGE( STATUS "Translation set: ${TRANSLATION_SET}" )

        # For those not familiar with 'sed': the tarball might list './' and './*.po'.
        # We process just the '*.po' lines and strip off any leading './'.
        EXECUTE_PROCESS(
                COMMAND tar tfj ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATION_SET}
                COMMAND sed -n "/\\.po$/s%.*/%%p"
                COMMAND awk "{printf $1\";\"}"
                OUTPUT_VARIABLE TRANSLATION_SET_CONTENT
        )
        MESSAGE( STATUS "Translations: ${TRANSLATION_SET_CONTENT}" )

        # Create 'LANG.po's from po.tar.bz2
        ADD_CUSTOM_COMMAND(
                OUTPUT ${TRANSLATION_SET_CONTENT}
                COMMAND tar xfj ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATION_SET}
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATION_SET}
        )

        # LANG.po ->msgfmt-> LANG.gmo
        SET( _gmoFiles )
        FOREACH( _currentPoFile ${TRANSLATION_SET_CONTENT} )

                GET_FILENAME_COMPONENT( _lang ${_currentPoFile} NAME_WE )
                SET( _gmoFile "${_lang}.gmo" )
                SET( _gmoFiles ${_gmoFiles} ${_gmoFile} )

                ADD_CUSTOM_COMMAND(
                        OUTPUT ${_gmoFile}
                        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_currentPoFile}
                        DEPENDS ${_currentPoFile}
                )

                INSTALL(
                        FILES ${CMAKE_CURRENT_BINARY_DIR}/${_gmoFile}
                        DESTINATION share/locale/${_lang}/LC_MESSAGES
                        RENAME ${_translation_set_basename}.mo
                )

                # the docs claim it can handle a list, but
                SET_DIRECTORY_PROPERTIES( PROPERTIES
                        ADDITIONAL_MAKE_CLEAN_FILES ${_currentPoFile}
                        ADDITIONAL_MAKE_CLEAN_FILES ${_gmoFile}
                )

        ENDFOREACH( _currentPoFile )

        # build all .gmo files
        ADD_CUSTOM_TARGET(
                translations ALL
                DEPENDS ${_gmoFiles}
        )

ENDMACRO( GETTEXT_CREATE_TARBALL_TRANSLATIONS )

#
# Macro to be called if .po files are part of the source tree.
#
MACRO(GETTEXT_CREATE_TRANSLATIONS _moBasename _firstPoFile)

   SET(_gmoFiles)

   SET(_addToAll)
   IF(${_firstPoFile} STREQUAL "ALL")
      SET(_addToAll "ALL")
      SET(_firstPoFile)
   ENDIF(${_firstPoFile} STREQUAL "ALL")

   FOREACH (_currentPoFile ${_firstPoFile} ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
      GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_gmoFile}
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absFile}
      )

      INSTALL(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_moBasename}.mo)
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(translations ${_addToAll} DEPENDS ${_gmoFiles})

ENDMACRO(GETTEXT_CREATE_TRANSLATIONS )

IF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND TRUE)
ELSE (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND FALSE)
   IF (GetText_REQUIRED)
      MESSAGE(FATAL_ERROR "GetText not found")
   ENDIF (GetText_REQUIRED)
ENDIF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
