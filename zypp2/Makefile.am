## Process this file with automake to produce Makefile.in
## ##################################################

SUBDIRS = cache

AM_CXXFLAGS = -DZYPP_BASE_LOGGER_LOGGROUP=\"zypp2\"

## ##################################################

pkginclude_HEADERS = 

## ##################################################

lib_LTLIBRARIES =	lib@PACKAGE@2.la

## ##################################################

lib@PACKAGE@2_la_SOURCES = 

lib@PACKAGE@_la_LDFLAGS =	@LIBZYPP_VERSION_INFO@

lib@PACKAGE@_la_LIBADD =        cache/lib@PACKAGE@2_cache.la \
				-lboost_filesystem \
				-lsqlite3 \
				-lutil
# !!! base/lib@PACKAGE@_base.la MUST BE THE LAST zypp lib linked

## ##################################################

EXTRA_DIST = @DOXYGEN/*

#<INSTALL-HEADER-TARGET>

# Automatically inserted by insert_install_target
# DO NOT REMOVE THE TAGS!

# Install header files only if different to version already installed -
# prevent unnecessary installations to keep make dependency changes
# and thus rebuilds for dependent packages as low as possible.

install-pkgincludeHEADERS: $(pkginclude_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgincludedir)
	@list='$(pkginclude_HEADERS)';							\
	for p in $$list; do								\
		if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; 			\
		if ! cmp -s $$d$$p $(DESTDIR)$(pkgincludedir)/$$p; then			\
			echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p";	\
			$(INSTALL_DATA) $$d$$p $(DESTDIR)$(pkgincludedir)/$$p;		\
	  	fi									\
	done

#</INSTALL-HEADER-TARGET>

