/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/Status.h
 *
*/
#ifndef ZYPP_UI_STATUS_H
#define ZYPP_UI_STATUS_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    /** UI status
     * Status values calculated by \ref Selectable.

     * \note The \ref Status enum, and it's use within \ref Selectabe
     * is tightly related to the Y2UI. It might be not verry usefull
     * outside the Y2UI.
     *
     * \todo make it an EnumerationClass
    */
    enum Status
    {
      S_Protected,           // Keep this unmodified ( have installedObj && S_Protected )
      S_Taboo,               // Keep this unmodified ( have no installedObj && S_Taboo)
      // requested by user:
      S_Del,                 // delete  installedObj ( clears S_Protected if set )
      S_Update,              // install candidateObj ( have installedObj, clears S_Protected if set )
      S_Install,             // install candidateObj ( have no installedObj, clears S_Taboo if set )
      // not requested by user:
      S_AutoDel,             // delete  installedObj
      S_AutoUpdate,          // install candidateObj ( have installedObj )
      S_AutoInstall,         // install candidateObj ( have no installedObj )
      // no modification:
      S_KeepInstalled,       // no modification      ( have installedObj && !S_Protected, clears S_Protected if set )
      S_NoInst,              // no modification      ( have no installedObj && !S_Taboo, clears S_Taboo if set )
    };

    ///////////////////////////////////////////////////////////////////

    /** \relates Status Enum value as string. */
    std::string asString( const Status & obj );

    ///////////////////////////////////////////////////////////////////

    /** \relates Status Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Status & obj )
    { return str << asString( obj ); }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_STATUS_H
