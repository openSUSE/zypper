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

///////////////////////////////////////////////////////////////////
/// \class FillSearchTableSolvable
/// \brief Functor for filling a detailed search output table.
///////////////////////////////////////////////////////////////////
struct FillSearchTableSolvable
{
  FillSearchTableSolvable( Table & table_r, TriBool instNotinst_r = indeterminate );

  /** Add this PoolItem if no filter applies */
  bool operator()( const PoolItem & pi_r ) const;
  /** Add this Solvable if no filter applies */
  bool operator()( const sat::Solvable & solv_r ) const;
  /** PoolQuery iterator provides info about matches */
  bool operator()( const PoolQuery::const_iterator & it_r ) const;

private:
  Table * _table;		//!< The table used for output
  std::set<std::string> _repos;	//!< Filter --repo
  TriBool _instNotinst;		//!< Filter --[not-]installed
};

struct FillSearchTableSelectable
{
  // the table used for output
  Table * _table;
  /** Aliases of repos specified as --repo */
  std::set<std::string> _repos;
  TriBool inst_notinst;

  FillSearchTableSelectable(
      Table & table, TriBool installed_only = indeterminate);

  bool operator()(const ui::Selectable::constPtr & s) const;
};

// struct FillPatchesTable		in src/utils/misc.h
// struct FillPatchesTableForIssue	in src/utils/misc.h


/** List all patches with specific info in specified repos */
void list_patches(Zypper & zypper);

/** List all patterns with specific info in specified repos */
void list_patterns(Zypper & zypper);

/** List all packages with specific info in specified repos
 *  - currently looks like zypper search -t package -r foorepo */
void list_packages(Zypper & zypper);

/** List all products with specific info in specified repos */
void list_products(Zypper & zypper);

#endif /*ZYPPERSEARCH_H_*/
