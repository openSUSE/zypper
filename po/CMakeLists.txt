# Provides the 'translations' target that creates the .gmo files
# out of the pofiles provided by zypp-po.tar.bz2.
# Use USE_TRANSLATION_SET to use an alternate set provided by
# ${USE_TRANSLATION_SET}-po.tar.bz2
#
GETTEXT_CREATE_TARBALL_TRANSLATIONS( "zypper" )
