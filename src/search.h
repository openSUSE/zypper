/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERSEARCH_H_
#define ZYPPERSEARCH_H_

#include <zypp/TriBool.h>
#include <zypp/PoolQuery.h>

#include "Zypper.h"
#include "Table.h"

//std::string selectable_search_repo_str(const zypp::ui::Selectable & s);

/**
 * Functor for filling search output table in rug style.
 */
struct FillSearchTableSolvable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;
  /** Aliases of repos specified as --repo */
  std::set<std::string> _repos;
  zypp::TriBool _inst_notinst;

  FillSearchTableSolvable(
      Table & table,
      zypp::TriBool inst_notinst = zypp::indeterminate );

  /** Add all items within this Selectable */
  bool operator()( const zypp::ui::Selectable::constPtr & sel ) const;
  /** Add this PoolItem */
  bool operator()( const zypp::PoolItem & pi ) const;
  /** Add this Solvable */
  bool operator()( zypp::sat::Solvable solv ) const;
  /** PoolQuery iterator provides info about matches*/
  bool operator()( const zypp::PoolQuery::const_iterator & it ) const;

  /** Helper to add a table row for \a sel's picklist item \c pi
   * \return whether a row was actually added.
   * \note picklist item means that \a pi must not be an installed
   * item in \a sel, if there is an identical available one. The
   * code relies on this.
   */
  bool addPicklistItem( const zypp::ui::Selectable::constPtr & sel, const zypp::PoolItem & pi ) const;
};

struct FillSearchTableSelectable
{
  // the table used for output
  Table * _table;
  const GlobalOptions & _gopts;
  /** Aliases of repos specified as --repo */
  std::set<std::string> _repos;
  zypp::TriBool inst_notinst;

  FillSearchTableSelectable(
      Table & table, zypp::TriBool installed_only = zypp::indeterminate);

  bool operator()(const zypp::ui::Selectable::constPtr & s) const;
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

  FillPatchesTable( Table & table,
      zypp::TriBool inst_notinst = zypp::indeterminate );

  bool operator()(const zypp::PoolItem & pi) const;
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
