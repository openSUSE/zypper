# Translation set name
SET( POT_NAME "zypp" )

SET( POT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${POT_NAME}.pot )

FILE( GLOB PO_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.po" )

# Provides the 'translations' target that
# creates the .gmo files out of the .po files
GETTEXT_CREATE_TRANSLATIONS( ${POT_FILE} "ALL" ${PO_FILES} )

# Creating the .pot file and preparing the .po files
# POT_FILE_DEPENDS was set in ../src
SET( XGETTEXT_CMD "xgettext" )
SET( XGETTEXT_OPTIONS -L C++ --boost --no-wrap --add-comments --add-location --foreign-user )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --keyword=_ --keyword=_:1,2 --keyword=__ --keyword=N_ --keyword=PL_:1,2 )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --copyright-holder=\"SuSE Linux GmbH, Nuernberg\" )
SET( XGETTEXT_OPTIONS ${XGETTEXT_OPTIONS} --package-name=${POT_NAME} --default-domain=${POT_NAME} )

ADD_CUSTOM_COMMAND( OUTPUT ${POT_FILE}
  COMMAND ${XGETTEXT_CMD} ${XGETTEXT_OPTIONS} --output=${POT_FILE} ${POT_FILE_DEPENDS}
  WORKING_DIRECTORY ${LIBZYPP_SOURCE_DIR}
  COMMENT "Extract translatable messages to ${POT_FILE}"
)

ADD_CUSTOM_TARGET( potfile
  COMMAND rm -f ${POT_FILE}
  COMMAND make translations
)
