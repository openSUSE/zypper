# - Find GNU gettext tools
# This module looks for the GNU gettext tools. This module defines the 
# following values:
#  GETTEXT_MSGMERGE_EXECUTABLE: the full path to the msgmerge tool.
#  GETTEXT_MSGFMT_EXECUTABLE: the full path to the msgfmt tool.
#  GETTEXT_FOUND: True if gettext has been found.
#
# Additionally it provides the following macros:
# GETTEXT_CREATE_TRANSLATIONS ( outputFile [ALL] file1 ... fileN )
#    This will create a target "translations" which will convert the 
#    given input po files into the binary output mo file. If the 
#    ALL option is used, the translations will also be created when
#    building the default target.

FIND_PROGRAM(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

MACRO(GETTEXT_CREATE_TRANSLATIONS _potFile _firstPoFile )

   SET(_gmoFiles)
   GET_FILENAME_COMPONENT(_potBasename ${_potFile} NAME_WE)
   GET_FILENAME_COMPONENT(_absPotFile ${_potFile} ABSOLUTE)

#   MESSAGE( STATUS "pot: ${_potFile} converted to ${_potBasename}")

   SET(_addToAll)
   IF(${_firstPoFile} STREQUAL "ALL")
      SET(_addToAll "ALL")
      SET(_firstPoFile)
   ENDIF(${_firstPoFile} STREQUAL "ALL")

   FOREACH (_currentPoFile ${ARGN})
      GET_FILENAME_COMPONENT(_absFile ${_currentPoFile} ABSOLUTE)
      GET_FILENAME_COMPONENT(_abs_PATH ${_absFile} PATH)
      GET_FILENAME_COMPONENT(_lang ${_absFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)
 
      SET( updatedPos "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po" ${updatedPos} )
 
      ADD_CUSTOM_COMMAND( 
         OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po"
         COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -o "${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po" -s ${_absFile} ${_absPotFile}
         DEPENDS ${_potFile}
      )
      
      ADD_CUSTOM_COMMAND( 
         OUTPUT ${_gmoFile} 
         COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_absFile}
         DEPENDS ${_absPotFile} ${_absFile}
      )

      INSTALL(FILES ${_gmoFile} DESTINATION share/locale/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo) 
      SET(_gmoFiles ${_gmoFiles} ${_gmoFile})

   ENDFOREACH (_currentPoFile )

   ADD_CUSTOM_TARGET(translations ${_addToAll} DEPENDS ${_gmoFiles} ${updatedPos})
#ADD_CUSTOM_TARGET(update_translations ${_addToAll} DEPENDS ${updatedPos} )
ENDMACRO(GETTEXT_CREATE_TRANSLATIONS )

MACRO(UPDATE_TRANSLATIONS)
  FILE( GLOB NEW_PO_FILES ${CMAKE_BINARY_DIR}/po/*.po )
  FOREACH (currentPoFile ${NEW_PO_FILES})
    GET_FILENAME_COMPONENT( lang ${currentPoFile} NAME_WE)
    ADD_CUSTOM_COMMAND( 
         OUTPUT "${CMAKE_SOURCE_DIR}/po/${lang}.po"
         COMMAND ${CMAKE_COMMAND} -E copy_if_different ${currentPoFile} "${CMAKE_SOURCE_DIR}/po/${lang}.po"
         DEPENDS ${currentPoFile}
    )
  ENDFOREACH (currentPoFile )
  ADD_CUSTOM_TARGET(update_translations ${_addToAll} DEPENDS ${NEW_PO_FILES} )
ENDMACRO(UPDATE_TRANSLATIONS)

IF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND TRUE)
ELSE (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )
   SET(GETTEXT_FOUND FALSE)
   IF (GetText_REQUIRED)
      MESSAGE(FATAL_ERROR "GetText not found")
   ENDIF (GetText_REQUIRED)
ENDIF (GETTEXT_MSGMERGE_EXECUTABLE AND GETTEXT_MSGFMT_EXECUTABLE )