# Translation set name
SET( POT_NAME "zypper" )

SET( POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${POT_NAME}.pot )
SET( NEW_POT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${POT_NAME}.pot )

FILE( GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.po" )

# Provides the 'translations' target that
# creates the .gmo files out of the .po files
GETTEXT_CREATE_TRANSLATIONS( ${POT_FILE} "ALL" ${PO_FILES} )

# Creating the .pot file and preparing the .po files.
# POT_FILE_DEPENDS was set in ../src.
# Don't update for changed line numbers only.
SET( XGETTEXT_CMD "xgettext" )
SET( XGETTEXT_OPTIONS -L C++ --boost --no-wrap --add-comments --add-location --foreign-user )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --keyword=_ --keyword=_:1,2 --keyword=__ --keyword=N_ --keyword=PL_:1,2 )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --copyright-holder=\"SuSE Linux GmbH, Nuernberg\" )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --package-name=${POT_NAME} --default-domain=${POT_NAME} )

ADD_CUSTOM_COMMAND( OUTPUT ${NEW_POT_FILE}
  COMMAND ${XGETTEXT_CMD} ${XGETTEXT_OPTIONS} --output=${NEW_POT_FILE} ${POT_FILE_DEPENDS}
  WORKING_DIRECTORY ${ZYPPER_SOURCE_DIR}
  COMMENT "Extract translatable messages to ${NEW_POT_FILE}"
)

ADD_CUSTOM_COMMAND( OUTPUT ${POT_FILE}
  COMMAND diff -u -I ^\#: -I ^.POT-Creation-Date: ${POT_FILE} ${NEW_POT_FILE} || cp -v ${NEW_POT_FILE} ${POT_FILE} && touch -t 200001010000 ${NEW_POT_FILE}
  DEPENDS ${NEW_POT_FILE}
  COMMENT "Update translatable messages in ${POT_FILE}"
)

ADD_CUSTOM_TARGET( potfile
  COMMAND rm -f ${NEW_POT_FILE}
  COMMAND make translations
)
