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
#include <zypp/base/Flags.h>

#include "Zypper.h"
#include "Table.h"
#include "utils/misc.h"

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
  /** For reverse dependency search */
  bool operator()( const sat::Solvable & solv_r, const sat::SolvAttr &searchedAttr, const CapabilitySet &matchedReq ) const;

private:
  std::string attribStr(const sat::SolvAttr &attr) const;

private:
  Table * _table;		//!< The table used for output
  std::set<std::string> _repos;	//!< Filter --repo
  TriBool _instNotinst;		//!< Filter --[not-]installed

};

struct FillSearchTableSelectable
{
  // the table used for output
  Table * _table;
  TriBool _instNotinst;
  bool _tagForeign;		//!< see NOTE in operator()

  FillSearchTableSelectable(
      Table & table, TriBool installed_only = indeterminate);

  bool operator()(const ui::Selectable::constPtr & s) const;
};

// struct FillPatchesTable		in src/utils/misc.h
// struct FillPatchesTableForIssue	in src/utils/misc.h


/** List all patches with specific info in specified repos */
void list_patches(Zypper & zypper);

/** List all patterns with specific info in specified repos */

void list_patterns(Zypper & zypper, SolvableFilterMode mode_r );

/** List all packages with specific info in specified repos
 *  - currently looks like zypper search -t package -r foorepo */
enum class ListPackagesBits {
  Default           = 0,
  HideInstalled     = 1 << 0,
  HideNotInstalled  = 1 << 1,
  ShowOrphaned      = 1 << 2,
  ShowSuggested     = 1 << 3,
  ShowRecommended   = 1 << 4,
  ShowUnneeded      = 1 << 5,
  SortByRepo        = 1 << 20  //< Result will be sorted by repo, not by name
};
ZYPP_DECLARE_FLAGS( ListPackagesFlags, ListPackagesBits );
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(ListPackagesFlags)
void list_packages(Zypper & zypper, ListPackagesFlags flags_r );

/** List all products with specific info in specified repos */
void list_products_xml( Zypper & zypper, SolvableFilterMode mode_r, const std::vector<std::string> &fwdTags );
void list_product_table( Zypper & zypper, SolvableFilterMode mode_r );


#endif /*ZYPPERSEARCH_H_*/
