
if(GLIB_INCLUDE_DIR AND GLIB_LIBRARY AND GLIB_CONFIG_INCLUDE_DIR)
	# Already in cache, be silent
	set(GLIB_FIND_QUIETLY TRUE)	
endif(GLIB_INCLUDE_DIR AND GLIB_LIBRARY AND GLIB_CONFIG_INCLUDE_DIR)

set(GLIB_LIBRARY)
set(GLIB_INCLUDE_DIR)
set(GLIB_CONFIG_INCLUDE_DIR)

FIND_PATH(GLIB_INCLUDE_DIR glib.h
	/usr/include
	/usr/include/glib-2.0
	/opt/gnome/include/glib-2.0
	/usr/local/include
)

FIND_PATH(GLIB_CONFIG_INCLUDE_DIR glibconfig.h
  /opt/gnome/lib/glib-2.0/include
  /usr/lib/glib-2.0/include
  /opt/gnome/lib64/glib-2.0/include
  /usr/lib64/glib-2.0/include
)

FIND_LIBRARY(GLIB_LIBRARY NAMES glib glib-2.0
	PATHS
	/usr/lib
	/opt/gnome/lib
	/usr/local/lib
)

if(GLIB_INCLUDE_DIR AND GLIB_LIBRARY AND GLIB_CONFIG_INCLUDE_DIR)
   MESSAGE( STATUS "glib found: includes in ${GLIB_INCLUDE_DIR}, library in ${GLIB_LIBRARY}")
   set(GLIB_FOUND TRUE)
else(GLIB_INCLUDE_DIR AND GLIB_LIBRARY AND GLIB_CONFIG_INCLUDE_DIR)
   MESSAGE( STATUS "glib not found")
endif(GLIB_INCLUDE_DIR AND GLIB_LIBRARY AND GLIB_CONFIG_INCLUDE_DIR)

MARK_AS_ADVANCED(GLIB_INCLUDE_DIR GLIB_LIBRARY GLIB_CONFIG_INCLUDE_DIR)