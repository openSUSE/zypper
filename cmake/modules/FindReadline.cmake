# - Find readline library
# This module looks for the GNU gettext tools. This module defines the
# following values:
#

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
        # Already in cache, be silent
        set(READLINE_FIND_QUIETLY TRUE)
endif(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

set(READLINE_LIBRARY)
set(READLINE_INCLUDE_DIR)

FIND_PATH(READLINE_INCLUDE_DIR readline/readline.h
  /usr/include
  /usr/include/readline
  /usr/local/include
  /usr/include/readline
)

# make find_library look only for shared lib
# otherwise it would find the static libreadline.a 
SET(CMAKE_FIND_LIBRARY_SUFFIXES_BACKUP ${CMAKE_FIND_LIBRARY_SUFFIXES})
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
FIND_LIBRARY(READLINE_LIBRARY readline
  PATHS
  /usr/lib
  /usr/lib64
  /usr/local/lib
  /usr/local/lib64
)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_BACKUP})

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
   MESSAGE( STATUS "readline found: includes in ${READLINE_INCLUDE_DIR}, library in ${READLINE_LIBRARY}")
   set(READLINE_FOUND TRUE)
else(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
   MESSAGE( STATUS "readline not found")
endif(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

MARK_AS_ADVANCED(READLINE_INCLUDE_DIR READLINE_LIBRARY)

