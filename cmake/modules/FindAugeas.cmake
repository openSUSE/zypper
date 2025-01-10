# Find augeas library and tool
#

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

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  Augeas
  REQUIRED_VARS AUGEAS_INCLUDE_DIR AUGEAS_LIBRARY)

MARK_AS_ADVANCED(AUGEAS_INCLUDE_DIR AUGEAS_LIBRARY)
