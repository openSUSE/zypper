/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPPERSEARCH_H_
#define ZYPPERSEARCH_H_

#include "zypp/ZYpp.h" // for zypp::ResPool::instance()
#include "zypp/sat/Solvable.h"
#include "zypp/PoolItem.h"

#include "zypper.h"
#include "zypper-utils.h" // for kind_to_string_localized
#include "zypper-tabulator.h"

/**
 * Functor for filling search output table in rug style.
 */
struct FillTable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;

  FillTable( Table & table )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  {
    TableHeader header;

    // TranslatorExplanation S as Status
    header << _("S");

    if (_gopts.is_rug_compatible)
      header << _("Catalog");
    else
      header << _("Repository");

    if (_gopts.is_rug_compatible)
      // TranslatorExplanation This is Bundle in as used in rug.
      header << _("Bundle");
    else
      header << _("Type");

    header << _("Name") << _("Version") << _("Arch");

    *_table << header;
  }

  bool operator()(const zypp::sat::Solvable & solv) const
    {
      TableRow row;

      // add other fields to the result table

      zypp::PoolItem pi( zypp::ResPool::instance().find( solv ) );

      row << ( pi.status().isInstalled() ? "i" : " " )
            << pi->repository().info().name()
      // TODO what about rug's Bundle?
      << (_gopts.is_rug_compatible ?
          "" : kind_to_string_localized(pi->kind(), 1))
              << pi->name()
              << pi->edition().asString()
              << pi->arch().asString();
          *_table << row;
      return true;
    }
};

#endif /*ZYPPERSEARCH_H_*/
