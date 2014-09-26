prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@LIB_INSTALL_DIR@/zypp
libdir=@LIB_INSTALL_DIR@
includedir=@INCLUDE_INSTALL_DIR@

Name: @PACKAGE@
Version: @VERSION@
Description: Package, Patch, Pattern, and Product Management

Libs: -L${libdir} -lzypp
Cflags: -I${includedir} @ZYPP_CFLAGS@

features=@ZYPP_FEATURES@

