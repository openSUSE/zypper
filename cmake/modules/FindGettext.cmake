# Try to find Gettext functionality
# Once done this will define
#
#  GETTEXT_FOUND - system has Gettext
#  GETTEXT_INCLUDE_DIR - Gettext include directory
#  GETTEXT_LIBRARIES - Libraries needed to use Gettext

# TODO: This will enable translations only if Gettext functionality is
# present in libc. Must have more robust system for release, where Gettext
# functionality can also reside in standalone Gettext library, or the one
# embedded within kdelibs (cf. gettext.m4 from Gettext source).

if (LIBC_HAS_DGETTEXT OR LIBINTL_HAS_DGETTEXT)

  # in cache already
  SET(GETTEXT_FOUND TRUE)

else (LIBC_HAS_DGETTEXT OR LIBINTL_HAS_DGETTEXT)

  include(CheckIncludeFiles)
  include(CheckLibraryExists)
  include(CheckFunctionExists)
  
  check_include_files(libintl.h HAVE_LIBINTL_H)
  
  set(GETTEXT_INCLUDE_DIR)
  set(GETTEXT_LIBRARIES)
  
  if (HAVE_LIBINTL_H)
     check_function_exists(dgettext LIBC_HAS_DGETTEXT)
     if (LIBC_HAS_DGETTEXT)
        set(GETTEXT_SOURCE "built in libc")
        set(GETTEXT_FOUND TRUE)
     else (LIBC_HAS_DGETTEXT)
        FIND_LIBRARY(LIBINTL_LIBRARY NAMES intl libintl
           PATHS
           /usr/lib
           /usr/local/lib
        )
        CHECK_LIBRARY_EXISTS(${LIBINTL_LIBRARY} "dgettext" "" LIBINTL_HAS_DGETTEXT)
        if (LIBINTL_HAS_DGETTEXT)
           set(GETTEXT_SOURCE "in ${LIBINTL_LIBRARY}")
           set(GETTEXT_LIBRARIES ${LIBINTL_LIBRARY} CACHE FILEPATH "path to libintl library, used for gettext")
           set(GETTEXT_FOUND TRUE)
        endif (LIBINTL_HAS_DGETTEXT)
     endif (LIBC_HAS_DGETTEXT)
  endif (HAVE_LIBINTL_H)
  
  if (GETTEXT_FOUND)
     if (NOT Gettext_FIND_QUIETLY)
        message(STATUS "Found Gettext: ${GETTEXT_SOURCE}")
     endif (NOT Gettext_FIND_QUIETLY)
  else (GETTEXT_FOUND)
     if (Gettext_FIND_REQUIRED)
        message(STATUS "Could NOT find Gettext")
     endif (Gettext_FIND_REQUIRED)
  endif (GETTEXT_FOUND)
  
  MARK_AS_ADVANCED(GETTEXT_INCLUDE_DIR GETTEXT_LIBRARIES)

endif (LIBC_HAS_DGETTEXT OR LIBINTL_HAS_DGETTEXT)
