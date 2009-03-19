# Find augeas library and tool
#

if(AUGEAS_INCLUDE_DIR AND AUGEAS_LIBRARY)
        # Already in cache, be silent
        set(AUGEAS_FIND_QUIETLY TRUE)
endif(AUGEAS_INCLUDE_DIR AND AUGEAS_LIBRARY)

set(AUGEAS_LIBRARY)
set(AUGEAS_INCLUDE_DIR)

FIND_PATH(AUGEAS_INCLUDE_DIR augeas.h
  /usr/include
  /usr/local/include
)

FIND_LIBRARY(AUGEAS_LIBRARY NAMES augeas
  PATHS
  /usr/lib
  /usr/lib64
  /usr/local/lib
  /usr/local/lib64
)

if(AUGEAS_INCLUDE_DIR AND AUGEAS_LIBRARY)
   MESSAGE( STATUS "augeas found: includes in ${AUGEAS_INCLUDE_DIR}, library in ${AUGEAS_LIBRARY}")
   set(AUGEAS_FOUND TRUE)
else(AUGEAS_INCLUDE_DIR AND AUGEAS_LIBRARY)
   MESSAGE( STATUS "augeas not found")
endif(AUGEAS_INCLUDE_DIR AND AUGEAS_LIBRARY)

MARK_AS_ADVANCED(AUGEAS_INCLUDE_DIR AUGEAS_LIBRARY)
