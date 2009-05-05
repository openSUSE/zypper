
if(RPM_INCLUDE_DIR AND RPM_LIBRARY)
	# Already in cache, be silent
	set(RPM_FIND_QUIETLY TRUE)
endif(RPM_INCLUDE_DIR AND RPM_LIBRARY)

set(RPM_LIBRARY)
set(RPM_INCLUDE_DIR)
set(RPM_4_4_LEGACY)

FIND_PATH(RPM_INCLUDE_DIR rpm/rpmdb.h
	/usr/include
	/usr/local/include
)

FIND_PATH(RPM_4_4_LEGACY rpm/rpmlegacy.h
	${RPM_INCLUDE_DIR}
	NO_DEFAULT_PATH
)

FIND_LIBRARY(RPM_LIBRARY NAMES rpm
	PATHS
	/usr/lib
	/usr/local/lib
)

if(RPM_INCLUDE_DIR AND RPM_LIBRARY)
   MESSAGE( STATUS "rpm found: includes in ${RPM_INCLUDE_DIR}, library in ${RPM_LIBRARY}")
   if ( RPM_4_4_LEGACY )
     MESSAGE( STATUS "rpm provides 4.4 legacy interface")
   endif ( RPM_4_4_LEGACY )
   set(RPM_FOUND TRUE)
else(RPM_INCLUDE_DIR AND RPM_LIBRARY)
   MESSAGE( STATUS "rpm not found")
endif(RPM_INCLUDE_DIR AND RPM_LIBRARY)

MARK_AS_ADVANCED(RPM_INCLUDE_DIR RPM_LIBRARY)