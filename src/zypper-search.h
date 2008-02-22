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

#include <string>
#include <vector>
#include <boost/function.hpp>
#include "zypp/ZYpp.h"
#include "zypp/PoolQuery.h"
#include "zypp/ResObject.h"

#include "zypper.h"
#include "zypper-main.h"
#include "zypper-utils.h"
#include "zypper-getopt.h"
#include "zypper-tabulator.h"

/**
 * Functor for filling search output table in rug style.
 */
struct FillTable
{
  // the table used for output
  Table * _table;
  // the db query interface, used to retrieve additional data like the repository alias
  zypp::PoolQuery _query;
  const GlobalOptions & _gopts;

  FillTable( Table & table, zypp::PoolQuery query )
  : _table( &table )
  , _query( query )
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

  // PoolItem callback, called for installed resolvables

  bool operator()(const zypp::ResObject::Ptr &item) const
  {
    //std::cout << "result" << std::endl;
    TableRow row;

    // add other fields to the result table
    row << "i"
	  << item->repository().info().name()
    // TODO what about rug's Bundle?
    << (_gopts.is_rug_compatible ?
        "" : kind_to_string_localized(item->kind(), 1))
            << item->name()
            << item->edition().asString()
            << item->arch().asString();
        *_table << row;
    return true;
  }
};

#endif /*ZYPPERSEARCH_H_*/
