/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERSEARCH_H_
#define ZYPPERSEARCH_H_

#include "zypp/ZYpp.h" // for zypp::ResPool::instance()
#include "zypp/sat/Solvable.h"
#include "zypp/PoolItem.h"
#include "zypp/Patch.h"
#include "zypp/TriBool.h"

#include "Zypper.h"
#include "utils/misc.h" // for kind_to_string_localized
#include "Table.h"

std::string selectable_search_repo_str(const zypp::ui::Selectable & s);
std::string string_patch_status(const zypp::PoolItem & pi);

/**
 * Functor for filling search output table in rug style.
 */
struct FillSearchTableSolvable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;
  zypp::TriBool _inst_notinst;

  FillSearchTableSolvable( Table & table, zypp::TriBool inst_notinst = zypp::indeterminate )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
  {
    TableHeader header;

    if (_gopts.is_rug_compatible)
    {
      header
        // translators: S for 'installed Status'
        << _("S")
        // translators: catalog (rug's word for repository) (header)
        << _("Catalog")
        // translators: Bundle is a term used in rug. See rug for how to translate it.
        << _("Bundle")
        // translators: name (general header)
        << _("Name")
        // translators: package version (header)
        << _("Version")
        // translators: package architecture (header)
        << _("Arch");
    }
    else
    {
      header
        // translators: S for 'installed Status'
        << _("S")
        // translators: name (general header)
        << _("Name")
        // translators: type (general header)
        << _("Type")
        // translators: package version (header)
        << _("Version")
        // translators: package architecture (header)
        << _("Arch")
        // translators: package's repository (header)
        << _("Repository");
    }

    *_table << header;
  }

  bool operator()(const zypp::ui::Selectable::constPtr & s) const
  {
    static bool show_installed;
    show_installed = true;

    // show available objects
    for_(it, s->availableBegin(), s->availableEnd())
    {
      TableRow row;

      zypp::PoolItem pi = *it;
      if (!s->installedEmpty())
      {
        static bool installed;
        installed = false;
        for_(instit, s->installedBegin(), s->installedEnd())
        {
          if (equalNVRA(*instit->resolvable(), *pi.resolvable()))
          {
            installed = true;
            show_installed = false;
            break;
          }
        }

        if (installed)
        {
          // show only not installed
          if (_inst_notinst == false)
            continue;
          row << "i";
        }
        else
        {
          // show only installed
          if (_inst_notinst == true)
            continue;
          row << "v";
        }
      }
      // patches/patterns are installed if satisfied
      else if (pi->kind() != zypp::ResKind::srcpackage && pi.isSatisfied()) 
      {
        // show only not installed
        if (_inst_notinst == false)
          continue;
        row << "i";
      }
      else
      {
        // show only installed
        if (_inst_notinst == true)
          continue;
        row << "";
      }

      if (_gopts.is_rug_compatible)
      {
        row
          << pi->repository().info().name()
          << ""
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();
      }
      else
      {
        row
          << pi->name()
          << kind_to_string_localized(pi->kind(), 1)
          << pi->edition().asString()
          << pi->arch().asString()
          << pi->repository().info().name();
      }

      *_table << row;
    }

    if (_inst_notinst == false)
      return true;

    // show installed objects only if there is no counterpart in repos
    if (show_installed || s->availableEmpty())
    {
      for_(it, s->installedBegin(), s->installedEnd())
      {
        TableRow row;
        zypp::PoolItem pi = *it;
        row << "i";
        if (_gopts.is_rug_compatible)
        {
          row
            << _("System Packages")
            // TODO what about rug's Bundle?
            << ""
            << pi->name()
            << pi->edition().asString()
            << pi->arch().asString();
        }
        else
        {
          row
            << pi->name()
            << kind_to_string_localized(pi->kind(), 1)
            << pi->edition().asString()
            << pi->arch().asString()
            << (string("(") + _("System Packages") + ")");
        }
  
        *_table << row;pi->repository().info().name();
      }
    }

    //! \todo mainain an internal plaindir repo named "Local Packages"
    // for plain rpms (as rug does). ?
    
    return true;
  }
};

struct FillSearchTableSelectable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;
  zypp::TriBool inst_notinst;

  FillSearchTableSelectable( Table & table, zypp::TriBool installed_only = zypp::indeterminate )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , inst_notinst(installed_only)
  {
    TableHeader header;
    // translators: S for installed Status
    header << _("S");
    header << _("Name");
    // translators: package summary (header)
    header << _("Summary");
    header << _("Type");
    *_table << header;
  }

  bool operator()(const zypp::ui::Selectable::constPtr & s) const
  {
    TableRow row;

    if (s->kind() != zypp::ResKind::srcpackage &&
        (!s->installedEmpty() || s->theObj().isSatisfied()))
    {
      // not-installed only
      if (inst_notinst == false)
        return true;
      row << "i";
    }
    else
    {
      // installed only
      if (inst_notinst == true)
        return true;
      row << "";
    }
    row << s->name();
    row << s->theObj()->summary();
    row << kind_to_string_localized(s->kind(), 1);
    *_table << row;
    return true;
  }
};


/**
 * Functor for filling search output table in rug style.
 */
struct FillPatchesTable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;
  zypp::TriBool _inst_notinst;
  
  FillPatchesTable( Table & table, zypp::TriBool inst_notinst = zypp::indeterminate )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
  {
    TableHeader header;

    header
      // translators: catalog (rug's word for repository) (header)
      << _("Catalog")
      << _("Name")
      << _("Version")
      // translators: patch category (recommended, security)
      << _("Category")
      // translators: patch status (installed, uninstalled, needed)
      << _("Status");

    *_table << header;
  }

  bool operator()(const zypp::PoolItem & pi) const
  {
    // only not installed
    if (pi.isSatisfied() && _inst_notinst == false)
      return true;
    // only installed
    else if (!pi.isSatisfied() && _inst_notinst == true)
      return true;

    TableRow row;

    zypp::Patch::constPtr patch = zypp::asKind<zypp::Patch>(pi.resolvable());
    
    row
      << pi->repository().info().name()
      << pi->name()
      << pi->edition().asString()
      << patch->category()
      << string_patch_status(pi);

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

/** List all products with specific info in specified repos */
void list_products(Zypper & zypper);

/** List all providers of given capability */
void list_what_provides(Zypper & zypper, const std::string & capstr);

#endif /*ZYPPERSEARCH_H_*/
