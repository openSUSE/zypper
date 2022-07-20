/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/SelFilters.h
 *
*/
#ifndef ZYPP_UI_SELFILTERS_H
#define ZYPP_UI_SELFILTERS_H

#include <string>

#include <zypp/base/Functional.h>
#include <zypp/ui/Selectable.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace selfilter
    { /////////////////////////////////////////////////////////////////

      /** */
      struct ByKind
      {
        ByKind( const ResKind & kind_r )
        : _kind( kind_r )
        {}

        bool operator()( const Selectable::constPtr & obj ) const
        {
          return obj && obj->kind() == _kind;
        }

        ResKind _kind;
      };

      /** */
      struct ByName
      {
        ByName( const std::string & name_r )
        : _name( name_r )
        {}

        bool operator()( const ui::Selectable::constPtr & obj ) const
        { return obj && obj->name() == _name; }

        std::string _name;
      };

      /** */
      struct ByHasInstalledObj
      {
        bool operator()( const ui::Selectable::constPtr & obj ) const
        { return obj && !obj->installedEmpty(); }
      };

      /** */
      struct ByHasCandidateObj
      {
        bool operator()( const ui::Selectable::constPtr & obj ) const
        { return obj && obj->hasCandidateObj(); }
      };

      struct ByStatus
      /** */
      {
        ByStatus( Status status_r )
        : _status( status_r )
        {}

        bool operator()( const ui::Selectable::constPtr & obj ) const
        { return obj && obj->status() == _status; }

        Status _status;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace selfilter
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELFILTERS_H
