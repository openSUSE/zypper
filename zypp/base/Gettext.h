/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Gettext.h
 *
 * Interface to gettext.
 *
*/
#ifndef ZYPP_BASE_GETTEXT_H
#define ZYPP_BASE_GETTEXT_H

#ifdef ZYPP_DLL //defined if zypp is compiled as DLL

/** Just tag text for translation. */
#undef N_
#define N_(MSG) MSG

/** Return translated text. */
#undef _
#define _(MSG) ::zypp::gettext::dgettext( MSG )

/** Return translated text (plural form). */
#undef PL_
#define PL_(MSG1,MSG2,N) ::zypp::gettext::dngettext( MSG1, MSG2, N )

#else

#ifndef N_
#define N_(MSG) MSG
#endif
#ifndef _
#define _(MSG) ::gettext( MSG )
#endif
#ifndef PL_
#define PL_(MSG1,MSG2,N) ::ngettext( MSG1, MSG2, N )
#endif

#endif

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace gettext
  { /////////////////////////////////////////////////////////////////

    /** Return translated text. */
    const char * dgettext( const char * msgid );

    /** Return translated text (plural form). */
    const char * dngettext( const char * msgid1, const char * msgid2,
                            unsigned long n );

    /////////////////////////////////////////////////////////////////
  } // namespace gettext
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_GETTEXT_H
