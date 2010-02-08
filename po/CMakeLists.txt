#
# Creating the .pot file to be translated.
#
ADD_CUSTOM_TARGET( zypp.pot
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/createPot ${LIBZYPP_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/zypp.pot
)
SET_DIRECTORY_PROPERTIES( PROPERTIES
	ADDITIONAL_MAKE_CLEAN_FILES zypp.pot
)

#
# Provides the 'translations' target that creates the .gmo files
# out of the pofiles provided by zypp-po.tar.bz2.
# Use USE_TRANSLATION_SET to use an alternate set provided by
# ${USE_TRANSLATION_SET}-po.tar.bz2
#
GETTEXT_CREATE_TARBALL_TRANSLATIONS( "zypp" )

