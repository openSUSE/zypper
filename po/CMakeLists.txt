# Translation set name
SET( POT_NAME "zypp" )

# Creating the .pot file....
# POT_FILE_DEPENDS was set in ../src
SET( POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${POT_NAME}.pot )

SET( XGETTEXT_CMD "xgettext" )
SET( XGETTEXT_OPTIONS -L C++ --boost --no-wrap --add-comments --add-location --foreign-user )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --keyword=_ --keyword=_:1,2 --keyword=__ --keyword=N_ --keyword=PL_:1,2 )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --copyright-holder=\"SuSE Linux GmbH, Nuernberg\" )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --package-name=${POT_NAME} --default-domain=${POT_NAME} )

SET( _abs_POT_FILE_DEPENDS )
FOREACH( _currentDepends ${POT_FILE_DEPENDS} )
  SET( _abs_POT_FILE_DEPENDS ${_abs_POT_FILE_DEPENDS} ${LIBZYPP_SOURCE_DIR}/${_currentDepends} )
ENDFOREACH()

ADD_CUSTOM_COMMAND( OUTPUT ${POT_FILE}
  COMMAND ${XGETTEXT_CMD} ${XGETTEXT_OPTIONS} --output=${POT_FILE} ${POT_FILE_DEPENDS}
  WORKING_DIRECTORY ${LIBZYPP_SOURCE_DIR}
  DEPENDS ${_abs_POT_FILE_DEPENDS}
  COMMENT "Extract translatable messages to ${POT_FILE}"
)


# Creating the .gmo files out of the .po files
FILE( GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.po" )

SET( GMO_FILES )
FOREACH( _currentPoFile ${PO_FILES} )
  GET_FILENAME_COMPONENT( _absFile ${_currentPoFile} ABSOLUTE )
  GET_FILENAME_COMPONENT( _lang ${_absFile} NAME_WE )

  SET( _poFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.po )
  SET( _gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo )

  ADD_CUSTOM_COMMAND( OUTPUT ${_gmoFile}
    COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet -o ${_poFile} ${_absFile} ${POT_FILE}
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} ${_poFile}
    DEPENDS ${POT_FILE} ${_absFile}
    COMMENT "Update ${_gmoFile}"
  )

  INSTALL( FILES ${_gmoFile}
    DESTINATION share/locale/${_lang}/LC_MESSAGES
    RENAME ${POT_NAME}.mo
  )

  SET( GMO_FILES ${GMO_FILES} ${_gmoFile} )
ENDFOREACH()


# Provides the 'translations' target that
# creates the .gmo files out of the .po files
SET( SOURCE_POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${POT_NAME}.pot )

ADD_CUSTOM_TARGET( translations ALL
  COMMAND diff -u -I ^\#: -I ^.POT-Creation-Date: ${SOURCE_POT_FILE} ${POT_FILE} | grep '^[+-][^\#]' || true
  DEPENDS ${GMO_FILES}
)

ADD_CUSTOM_TARGET( potfile_update
  COMMAND diff -q -I ^\#: -I ^.POT-Creation-Date: ${SOURCE_POT_FILE} ${POT_FILE}
       || ./PotfileUpadte.sh ${CMAKE_CURRENT_BINARY_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DEPENDS translations
  COMMENT "Checking for uncommitted changes to the .pot file..."
)
