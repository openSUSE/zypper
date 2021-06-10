/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <functional>
#include <string>
#include <set>
#include <list>

#include <zypp/Url.h>
#include <zypp/Date.h>
#include <zypp/Pathname.h>

#include <zypp/ui/Selectable.h>
#include <zypp/ZYppCommitPolicy.h>
#include <zypp/base/Logger.h>

class Zypper;
class Table;

namespace zypp
{
  class Resolvable;
  class Product;
  class Pattern;
  class Patch;
}
using namespace zypp;

typedef std::set<ResKind> ResKindSet;


/**
 * Used to specify if installed and not installed solvables
 * should be listed or just simply all of them
 */
enum class SolvableFilterMode {
  ShowAll,
  ShowOnlyInstalled,
  ShowOnlyNotInstalled
};

/** Whether the item is considered to be (i)nstalled
 *
 * ( installed or ( pseudoInstalled && satisfied ) )
 *
 * Sometimes used to pre-filter items on ui::Selectable level to
 * avoid unnecessary processing of the picklist items.
 */
bool iType( const ui::Selectable::constPtr & sel_r );

/*!
 * Computes the status indicator for a given \ref PoolItem
 *   i  - exactly this version installed (or satisfied Patch)
 *   v  - installed, but in a different version
 *      - not installed at all
 *   !  - broken/needed Patch
 *
 *  Adds additional suffices for additional information:
 *   P  - The item is part of a PTF
 *   R  - The item has been retracted
 *   l  - The item is locked
 *   +  - Only for the "i" status, the item is user installed
 *
 *  - If you happen to have the corresponding Selectable::Ptr at hand,
 *    pass it down. Otherwise we'll look it up.
 *  - May on the fly return whether the item is treated as (i)nstalled.
 *    ( installed or ( pseudoInstalled && satisfied ) ).
 */
const char * computeStatusIndicator( const PoolItem & pi_r, ui::Selectable::constPtr sel_r = nullptr, bool * iType_r = nullptr );
/** \overload */
inline const char * computeStatusIndicator( const PoolItem & pi_r, bool * iType_r )
{ return computeStatusIndicator( pi_r, nullptr, iType_r ); }

/*!
 * Computes the status indicator for a given \ref ui::Selectable
 *   i  - There is an installed item
 *   v  - IFF tagForeign_r==true: There is an installed item, but it's not from a known repo.
 *      - not installed at all
 *
 *  Adds additional suffices for additional information:
 *   P  - The item is part of a PTF
 *   R  - The item has been retracted
 *   l  - The item is locked
 *   +  - Only for the "i/v" status, the item is user installed
 *
 *  - May on the fly return whether the item is treated as (i)nstalled.
 *    ( installed or ( pseudoInstalled && satisfied ) ).
 */
const char * computeStatusIndicator( const ui::Selectable & sel_r, bool tagForeign_r = false, bool * iType_r = nullptr );
/** \overload */
inline const char * computeStatusIndicator( const ui::Selectable & sel_r, bool * iType_r )
{ return computeStatusIndicator( sel_r, false, iType_r ); }
/** \overload */
inline const char * computeStatusIndicator( const ui::Selectable & sel_r, const Edition &installedMustHaveEd )
{ return computeStatusIndicator( sel_r ); }


/** Whether running on SLE.
 * If so, report e.g. unsupported packages per default.
 */
bool runningOnEnterprise();

/** Converts user-supplied kind to ResKind object.
 * Returns an empty one if not recognized. */
ResKind string_to_kind( std::string skind );

ResKindSet kindset_from( const std::list<std::string> & kindstrings );

std::string kind_to_string_localized( const ResKind & kind, unsigned long count );


// ----------------------------------------------------------------------------
// PATCH related strings for various purposes
// ----------------------------------------------------------------------------
const char* textPatchStatus( const PoolItem & pi_r );		///< Patch status: plain text noWS (forXML)
std::string i18nPatchStatus( const PoolItem & pi_r );		///< Patch status: i18n + color
std::string patchHighlightCategory( const Patch & patch_r );	///< Patch Category + color
std::string patchHighlightSeverity( const Patch & patch_r );	///< Patch Severity + color
std::string patchInteractiveFlags( const Patch & patch_r );	///< Patch interactive properties (reboot|message|license|restart or ---) + color

///////////////////////////////////////////////////////////////////
/// Last Patch status changes parsed from the history file.
/// If these data are made available to \ref FillPatchesTable a
/// \c Since column is shown.
class PatchHistoryData
{
public:
  using value_type = std::pair<Date,ResStatus::ValidateValue>;

  static const value_type noData;	///< Date() and ResStatus::UNDETERMINED

  /** Ctor parsing the history file. */
  PatchHistoryData() : PatchHistoryData( true ) {}

  /** Return an empty instance without data. */
  static PatchHistoryData placeholder();

  /** Whether patch status data are available at all. */
  explicit operator bool() const;

  /** Return the last status change of \a pi_r or \ref noData. */
  const value_type & operator[]( const sat::Solvable & solv_r ) const;
  /** \overload */
  const value_type & operator[]( const PoolItem & pi_r ) const
  { return  operator[]( pi_r.satSolvable() ); }

private:
  PatchHistoryData( bool );
  struct D;
  RW_pointer<D> _d;
};


/** Patches table default format */
struct FillPatchesTable
{
  FillPatchesTable( Table & table_r, const PatchHistoryData & historyData_r, TriBool inst_notinst_r = indeterminate );
  bool operator()( const PoolItem & pi_r ) const;
private:
  std::reference_wrapper<Table> _table;	///< table used for output.
  TriBool _inst_notinst;	///< LEGACY: internally filter [not]installed.
  PatchHistoryData _historyData;///< optional history data pass to the ctor.
};

/** Patches table when searching for issues */
struct FillPatchesTableForIssue
{
  FillPatchesTableForIssue( Table & table_r, const PatchHistoryData & historyData_r );
  bool operator()( const PoolItem & pi_r, std::string issue_r, std::string issueNo_r ) const;
private:
  std::reference_wrapper<Table> _table;	///< table used for output
  PatchHistoryData _historyData;///< optional history data pass to the ctor.
};

// ----------------------------------------------------------------------------
/**
 * Creates a Url out of \a urls_s. If the url_s looks looks_like_url()
 * Url(url_s) is returned. Otherwise if \a url_s represends a valid path to
 * a file or directory, a dir:// Url is returned. Otherwise an empty Url is
 * returned.
 */
Url make_url( const std::string & url_s );

/**
 * Creates Url out of obs://project/platform URI with given base URL and default
 * platform (used in case the platform is not specified in the URI).
 */
Url make_obs_url( const std::string & obsuri );

/**
 * Returns <code>true</code> if the string \a s contains a substring starting
 * at the beginning and ending before the first colon matches any of registered
 * schemes (Url::isRegisteredScheme()).
 */
bool looks_like_url( const std::string& s );

/**
 * Returns <code>true</code> if \a s ends with ".rpm" or starts with "/", "./",
 * or "../".
 */
bool looks_like_rpm_file( const std::string & s );

/**
 * Download the RPM file specified by \a rpm_uri_str and copy it into
 * \a cache_dir.
 *
 * \return The local Pathname of the file in the cache on success, empty
 *      Pathname if a problem occurs.
 */
Pathname cache_rpm( const std::string & rpm_uri_str, const Pathname & cache_dir );

std::string & indent( std::string & text, int columns );

// comparator for RepoInfo set
struct RepoInfoAliasComparator
{
  bool operator()( const RepoInfo & a, const RepoInfo & b ) const
  { return a.alias() < b.alias(); }
};


// comparator for Service set
struct ServiceAliasComparator
{
  bool operator()( const repo::RepoInfoBase_Ptr & a, const repo::RepoInfoBase_Ptr & b ) const
  { return a->alias() < b->alias(); }
};


/**
 * checks name for .repo string
 */
inline bool isRepoFile( const std::string & name )
{ return name.find(".repo") != name.npos; }

std::string asXML( const Product & p, bool is_installed, const std::vector<std::string> &fwdTags = {} );

std::string asXML( const Pattern & p, bool is_installed );

/** Check whether packagekit is running using a DBus call */
bool packagekit_running();

/** Send suggestion to quit to PackageKit via DBus */
void packagekit_suggest_quit();

#endif /*ZYPPER_UTILS_H*/
