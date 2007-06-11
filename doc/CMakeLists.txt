INSTALL(
  FILES zypper.8
  DESTINATION ${MANDIR}/man8
)

INSTALL(
  FILES TODO zypper-rug
  DESTINATION ${INSTALL_PREFIX}/share/doc/packages/${PACKAGE}
)

# old Makefile.am
#pkgdoc_DATA = TODO zypper-rug
#man_MANS = zypper.8
#
#EXTRA_DIST = $(man_MANS) $(pkgdoc_DATA)
