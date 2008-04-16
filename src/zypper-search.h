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

std::string selectable_search_repo_str(const zypp::ui::Selectable & s);

/**
 * Functor for filling search output table in rug style.
 */
struct FillSearchTableSolvable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;

  FillSearchTableSolvable( Table & table )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  {
    TableHeader header;

    // translators: S for installed Status
    header << _("S");

    if (_gopts.is_rug_compatible)
      header << _("Catalog");
    else
      header << _("Repository");

    if (_gopts.is_rug_compatible)
      // translators: Bundle is a term used in rug. See rug for how to translate it.
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

  bool operator()(const zypp::ui::Selectable::constPtr & s) const
  {
    // show installed objects
    for_(it, s->installedBegin(), s->installedEnd())
    {
      TableRow row;
      zypp::PoolItem pi = *it;
      row << "i";
      row << pi->repository().info().name()
          // TODO what about rug's Bundle?
          << (_gopts.is_rug_compatible ? "" : kind_to_string_localized(pi->kind(), 1))
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();
      *_table << row;
    }

    // get the first installed object
    zypp::PoolItem installed;
    if (!s->installedEmpty())
      installed = s->installedObj();

    // show available objects
    for_(it, s->availableBegin(), s->availableEnd())
    {
      TableRow row;

      zypp::PoolItem pi = *it;
      if (installed)
      {
        row << (equalNVRA(*installed.resolvable(), *pi.resolvable()) ?  "i" : "v");
      }
      else
        row << "";
      row << pi->repository().info().name();
      row << (_gopts.is_rug_compatible ? "" : kind_to_string_localized(pi->kind(), 1))
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();

      *_table << row;
    }
    return true;
  }
};

struct FillSearchTableSelectable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;

  FillSearchTableSelectable( Table & table )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  {
    TableHeader header;
    // translators: S for installed Status
    header << _("S");
    header << _("Name");
    header << _("Summary");
    header << _("Type");
    header << _("Repository");
    *_table << header;
  }

  bool operator()(const zypp::ui::Selectable::constPtr & s) const
  {
    TableRow row;

    row << (s->installedEmpty() ? "" : "i");
    //! \todo improve the abbreviation
    row << (_gopts.no_abbrev || s->theObj()->summary().size() < 33 ?
        s->theObj()->summary() : s->theObj()->summary().substr(0,28) + "...");
    row << kind_to_string_localized(s->kind(), 1);
    row << selectable_search_repo_str(*s);
    *_table << row;
    return true;
  }
};

/** List all patches with specific info in specified repos */
void list_patches(Zypper & zypper);

/** List all patterns with specific info in specified repos */
void list_patterns(Zypper & zypper);

/** List all packages with specific info in specified repos
 *  - currently looks like zypper search -t package -r foorepo */
void list_packages(Zypper & zypper);

#endif /*ZYPPERSEARCH_H_*/
