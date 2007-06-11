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

FIND_LIBRARY(READLINE_LIBRARY NAMES readline
  PATHS
  /usr/lib
  /usr/lib64
  /usr/local/lib
  /usr/local/lib64
)

if(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
   MESSAGE( STATUS "readline found: includes in ${READLINE_INCLUDE_DIR}, library in ${READLINE_LIBRARY}")
   set(READLINE_FOUND TRUE)
else(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)
   MESSAGE( STATUS "readline not found")
endif(READLINE_INCLUDE_DIR AND READLINE_LIBRARY)

MARK_AS_ADVANCED(READLINE_INCLUDE_DIR READLINE_LIBRARY)

