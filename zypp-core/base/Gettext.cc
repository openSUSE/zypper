/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Gettext.cc
 *
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern "C" {
#include <libintl.h>
}

#include <zypp-core/base/Gettext.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace gettext
  { /////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
    // TEXTDOMAIN and LOCALEDIR must be provided via config.h
    // or at compile time using -D.
    /////////////////////////////////////////////////////////////////

    inline void assertInit()
    {
      static bool initialized = false;
      if ( ! initialized )
        {
          ::bindtextdomain( TEXTDOMAIN, LOCALEDIR );
          ::bind_textdomain_codeset( TEXTDOMAIN, "UTF-8" );
          initialized = true;
        }
    }

    const char * dgettext( const char * msgid )
    {
      assertInit();
      return ::dgettext( TEXTDOMAIN, msgid );
    }

    const char * dngettext( const char * msgid1, const char * msgid2,
                            unsigned long n )
    {
      assertInit();
      return ::dngettext( TEXTDOMAIN, msgid1, msgid2, n );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace gettext
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
