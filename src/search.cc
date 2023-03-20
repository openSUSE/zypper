#include <iostream>

#include <zypp/ZYpp.h> // for ResPool::instance()

#include <zypp/base/Logger.h>
#include <zypp/base/Algorithm.h>
#include <zypp/Patch.h>
#include <zypp/Pattern.h>
#include <zypp/Product.h>
#include <zypp/sat/Solvable.h>

#include <zypp/PoolItem.h>
#include <zypp/PoolQuery.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/ui/SelectableTraits.h>

#include "main.h"
#include "utils/misc.h"
#include "global-settings.h"

#include "search.h"

extern ZYpp::Ptr God;

///////////////////////////////////////////////////////////////////
// class FillSearchTableSolvable
///////////////////////////////////////////////////////////////////

FillSearchTableSolvable::FillSearchTableSolvable( Table & table_r, TriBool instNotinst_r )
: _table( &table_r )
, _instNotinst( instNotinst_r )
{
  Zypper & zypper( Zypper::instance() );
  if ( InitRepoSettings::instance()._repoFilter.size() )
  {
    for ( const auto & ri : zypper.runtimeData().repos )
      _repos.insert( ri.alias() );
  }

  //
  // *** CAUTION: It's a mess, but adding/changing colums here requires
  //              adapting OutXML::searchResult !
  //
  *_table << ( TableHeader()
          // translators: S for 'installed Status'
          << N_("S")
          // translators: name (general header)
          << table::Column( N_("Name"), table::CStyle::SortCi )
          // translators: type (general header)
          << N_("Type")
          // translators: package version (header)
          << table::Column( N_("Version"), table::CStyle::Edition )
          // translators: package architecture (header)
          << N_("Arch")
          // translators: package's repository (header)
          << N_("Repository") );
}

bool FillSearchTableSolvable::operator()( const PoolItem & pi_r ) const
{
  // --repo => we only want the repo resolvables, not @System (bnc #467106)
  if ( !_repos.empty() && !_repos.count( pi_r.repoInfo().alias() ) )
    return false;

  // hide patterns with user visible flag not set (bnc #538152)
  if ( pi_r->isKind<Pattern>() && ! pi_r->asKind<Pattern>()->userVisible() )
    return false;

  // NOTE The query delivers available items even if _instNotinst == true.
  // That's why we can/must discard installed items, if an identical available
  // is present. Most probably done to get the correct repo. (but not efficient).
  // As we need the picklistPos anyway we can use it to dicard picklistNoPos ones.
  ui::Selectable::Ptr sel { ui::Selectable::get( pi_r ) };
  ui::Selectable::picklist_size_type picklistPos { sel->picklistPos( pi_r ) };

  if ( picklistPos == ui::Selectable::picklistNoPos )
    return false;

  // On the fly filter unwanted according to _instNotinst
  const char *statusIndicator = nullptr;
  if ( indeterminate(_instNotinst)  )
    statusIndicator = computeStatusIndicator( pi_r, sel );
  else
  {
    bool iType;
    statusIndicator = computeStatusIndicator( pi_r, sel, &iType );
    if ( (bool)_instNotinst != iType )
      return false;
  }

  TableRow row;
  row
    << statusIndicator
    << pi_r->name()
    << kind_to_string_localized( pi_r->kind(), 1 )
    << pi_r->edition().asString()
    << pi_r->arch().asString()
    << ( pi_r->isSystem()
       ? (std::string("(") + _("System Packages") + ")")
       : pi_r->repository().asUserString() );

  row.userData( SolvableCSI(pi_r.satSolvable(), picklistPos) );

  *_table << std::move(row);

  return true;	// actually added a row
}

bool FillSearchTableSolvable::operator()( const sat::Solvable & solv_r ) const
{ return operator()( PoolItem( solv_r ) ); }

bool FillSearchTableSolvable::operator()( const PoolQuery::const_iterator & it_r ) const
{
  if ( ! operator()(*it_r) )
    return false;	// no row was added due to filter

  // add the details about matches to last row
  TableRow & lastRow( _table->rows().back() );

  // don't show details for patterns with user visible flag not set (bnc #538152)
  if ( it_r->kind() == ResKind::pattern )
  {
    Pattern::constPtr ptrn = asKind<Pattern>(*it_r);
    if ( ptrn && !ptrn->userVisible() )
      return true;
  }

  if ( !it_r.matchesEmpty() )
  {
    for_( match, it_r.matchesBegin(), it_r.matchesEnd() )
    {
      std::string attrib = attribStr( match->inSolvAttr() );
      if ( match->inSolvAttr() == sat::SolvAttr::summary ||
           match->inSolvAttr() == sat::SolvAttr::description )
      {
        // multiline matchstring
        lastRow.addDetail( attrib + ":" );
        lastRow.addDetail( match->asString() );
      }
      else
      {
        // print attribute and match in one line, e.g. requires: libzypp >= 11.6.2
        lastRow.addDetail( attrib + ": " + match->asString() );
      }
    }
  }
  return true;
}


std::string FillSearchTableSolvable::attribStr(  const sat::SolvAttr &attr  ) const
{
  std::string attrib( attr.asString() );
  if ( str::startsWith( attrib, "solvable:" ) )	// strip 'solvable:' from attribute
    attrib.erase( 0, 9 );
  return attrib;
}


bool FillSearchTableSolvable::operator()(const sat::Solvable &solv_r, const sat::SolvAttr &searchedAttr, const CapabilitySet &matchedAttribs ) const
{
  if ( ! operator()(solv_r) )
    return false;	// no row was added due to filter

  // add the details about matches to last row
  TableRow & lastRow( _table->rows().back() );

  // don't show details for patterns with user visible flag not set (bnc #538152)
  if ( solv_r.kind() == ResKind::pattern )
  {
    Pattern::constPtr ptrn = asKind<Pattern>(solv_r);
    if ( ptrn && !ptrn->userVisible() )
      return true;
  }

  auto attrStr = attribStr( searchedAttr );

  for ( const auto &cap : matchedAttribs ) {
    lastRow.addDetail( attrStr +": " + cap.asString() );
  }

  return true;
}

///////////////////////////////////////////////////////////////////

FillSearchTableSelectable::FillSearchTableSelectable( Table & table, TriBool installed_only )
: _table( &table )
, _instNotinst( installed_only )
, _tagForeign( InitRepoSettings::instance()._repoFilter.size() )
{
  //
  // *** CAUTION: It's a mess, but adding/changing colums here requires
  //              adapting OutXML::searchResult !
  //
  *_table << ( TableHeader()
          // translators: S for installed Status
          << N_("S")
          << table::Column( N_("Name"), table::CStyle::SortCi )
          // translators: package summary (header)
          << N_("Summary")
          << N_("Type") );
}

bool FillSearchTableSelectable::operator()( const ui::Selectable::constPtr & s ) const
{
  // NOTE _tagForeign: This is a legacy issue:
  // - 'zypper search' always shows installed items as 'i'
  // - 'zypper search --repo X' shows 'foreign' installed items (the installed
  //    version is not provided by one of the enabled repos) as 'v'.

  // hide patterns with user visible flag not set (bnc #538152)
  if ( s->kind() == ResKind::pattern && ! asKind<Pattern>(s->theObj())->userVisible() )
    return true;

  // On the fly filter unwanted according to _instNotinst
  const char *statusIndicator = nullptr;
  if ( indeterminate(_instNotinst)  )
    statusIndicator = computeStatusIndicator( *s, _tagForeign );
  else
  {
    bool iType;
    statusIndicator = computeStatusIndicator( *s, _tagForeign, &iType );
    if ( (bool)_instNotinst != iType )
      return true;
  }

  *_table << ( TableRow()
  << statusIndicator
  << s->name()
  << s->theObj()->summary()
  << kind_to_string_localized( s->kind(), 1 )
  );

  return true;
}

///////////////////////////////////////////////////////////////////

static std::string string_weak_status( const ResStatus & rs )
{
  if ( rs.isRecommended() )
    return _("Recommended");
  if ( rs.isSuggested() )
    return _("Suggested");
  return "";
}


void list_patches( Zypper & zypper )
{
  MIL << "Pool contains " << God->pool().size() << " items. Checking whether available patches are needed." << std::endl;

  Table tbl;
  FillPatchesTable callback( tbl, PatchHistoryData() );
  invokeOnEach( God->pool().byKindBegin(ResKind::patch),
                God->pool().byKindEnd(ResKind::patch),
                callback);

  if ( tbl.empty() )
    zypper.out().info( _("No needed patches found.") );
  else
  {
    // display the result, even if --quiet specified
    tbl.sort();	// use default sort
    cout << tbl;
  }
}

static void list_patterns_xml( Zypper & zypper, SolvableFilterMode mode_r )
{
  cout << "<pattern-list>" << endl;

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only   = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  for ( const auto & pi : God->pool().byKind<Pattern>() )
  {
    bool isInstalled = pi.status().isInstalled();
    if ( isInstalled && notinst_only && !installed_only )
      continue;
    if ( !isInstalled && installed_only && !notinst_only )
      continue;
    if ( repofilter && pi.repository().info().name() == "@System" )
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(pi.resolvable());
    cout << asXML( *pattern, isInstalled ) << endl;
  }

  cout << "</pattern-list>" << endl;
}

static void list_pattern_table( Zypper & zypper, SolvableFilterMode mode_r )
{
  MIL << "Going to list patterns." << std::endl;

  Table tbl;

  // translators: S for installed Status
  tbl << ( TableHeader()
      << N_("S")
      << table::Column( N_("Name"), table::CStyle::SortCi )
      << table::Column( N_("Version"), table::CStyle::Edition )
      << N_("Repository")
      << N_("Dependency") );

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only   = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  for( const auto & sel : God->pool().proxy().byKind<Pattern>() )
  {
    for ( const auto & pi : sel->picklist() )
    {
      bool isInstalled = pi.status().isInstalled() || sel->identicalInstalledObj( pi );
      if ( isInstalled && notinst_only && !installed_only )
        continue;
      else if ( !isInstalled && installed_only && !notinst_only )
        continue;

      const std::string & piRepoName( pi.repoInfo().name() );
      if ( repofilter && piRepoName == "@System" )
        continue;

      Pattern::constPtr pattern = asKind<Pattern>(pi.resolvable());
      // hide patterns with user visible flag not set (bnc #538152)
      if ( !pattern->userVisible() )
        continue;

      tbl << ( TableRow()
          << computeStatusIndicator( pi )
          << pi.name()
          << pi.edition()
          << piRepoName
          << string_weak_status(pi.status()) );
    }
  }
  tbl.sort( 1 ); // Name

  if ( tbl.empty() )
    zypper.out().info(_("No patterns found.") );
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_patterns(Zypper & zypper , SolvableFilterMode mode_r)
{
  if ( zypper.out().type() == Out::TYPE_XML )
    list_patterns_xml( zypper, mode_r );
  else
    list_pattern_table( zypper, mode_r );
}

void list_packages(Zypper & zypper , ListPackagesFlags flags_r )
{
  MIL << "Going to list packages." << std::endl;
  Table tbl;

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool showInstalled = !flags_r.testFlag( ListPackagesBits::HideInstalled ); //installed_only || !uninstalled_only;
  bool showUninstalled = !flags_r.testFlag( ListPackagesBits::HideNotInstalled ); //uninstalled_only || !installed_only;

  bool orphaned = flags_r.testFlag( ListPackagesBits::ShowOrphaned );
  bool suggested = flags_r.testFlag( ListPackagesBits::ShowSuggested );
  bool recommended = flags_r.testFlag( ListPackagesBits::ShowRecommended );
  bool unneeded = flags_r.testFlag( ListPackagesBits::ShowUnneeded );
  bool check = ( orphaned || suggested || recommended || unneeded );
  if ( check )
  {
    God->resolver()->resolvePool();
  }
  auto checkStatus = [=]( ResStatus status_r )->bool {
    return ( ( orphaned && status_r.isOrphaned() )
          || ( suggested && status_r.isSuggested() )
          || ( recommended && status_r.isRecommended() )
          || ( unneeded && status_r.isUnneeded() ) );
  };

  for( const auto & sel : God->pool().proxy().byKind<Package>() )
  {
    // filter on selectable level
    // legacy: unlike 'search -i', 'packages -i' lists all versions (i and v) IFF hasInstalled
    if ( iType( sel ) )
    {
      if ( ! showInstalled )
        continue;
    }
    else
    {
      if ( ! showUninstalled )
        continue;
    }

    for ( const auto & pi : sel->picklist() )
    {
      if ( check )
      {
        // if checks are more detailed, show only matches
        // not whole selectables
        if ( pi.status().isInstalled() )
        {
          if ( ! checkStatus( pi.status() ) )
            continue;
        }
        else
        {
          PoolItem ipi( sel->identicalInstalledObj( pi ) );
          if ( !ipi || !checkStatus( ipi.status() ) )
            if ( ! checkStatus( pi.status() ) )
              continue;
        }
      }

      const std::string & piRepoName( pi->repository().info().name() );
      if ( repofilter && piRepoName == "@System" )
        continue;

      tbl << ( TableRow()
          << computeStatusIndicator( pi, sel )
          << piRepoName
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString() );
    }
  }

  if ( tbl.empty() )
    zypper.out().info(_("No packages found.") );
  else
  {
    // display the result, even if --quiet specified
    tbl << ( TableHeader()
        // translators: S for installed Status
        << N_("S")
        << N_("Repository")
        << table::Column( N_("Name"), table::CStyle::SortCi )
        << table::Column( N_("Version"), table::CStyle::Edition )
        << N_("Arch") );

    if ( flags_r.testFlag( ListPackagesBits::SortByRepo ) )
      tbl.sort( 1 ); // Repo
    else
      tbl.sort( 2 ); // Name

    cout << tbl;
  }
}

void list_products_xml( Zypper & zypper, SolvableFilterMode mode_r, const std::vector<std::string> &fwdTags )
{
  bool repofilter =  InitRepoSettings::instance()._repoFilter.size();	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  cout << "<product-list>" << endl;
  for ( const auto & pi : God->pool().byKind<Product>() )
  {
    if ( pi.status().isInstalled() && notinst_only )
      continue;
    else if ( !pi.status().isInstalled() && installed_only )
      continue;
    if ( repofilter && pi.repository().info().name() == "@System" )
      continue;
    Product::constPtr product = asKind<Product>(pi.resolvable());
    cout << asXML( *product, pi.status().isInstalled(), fwdTags ) << endl;
  }
  cout << "</product-list>" << endl;
}

void list_product_table(Zypper & zypper , SolvableFilterMode mode_r)
{
  MIL << "Going to list products." << std::endl;

  Table tbl;

  tbl << ( TableHeader()
      // translators: S for installed Status
      << N_("S")
      << N_("Repository")
      // translators: used in products. Internal Name is the unix name of the
      // product whereas simply Name is the official full name of the product.
      << N_("Internal Name")
      << table::Column( N_("Name"), table::CStyle::SortCi )
      << table::Column( N_("Version"), table::CStyle::Edition )
      << N_("Arch")
      << N_("Is Base") );

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  for( const auto & sel : God->pool().proxy().byKind<Product>() )
  {
    for ( const auto & pi : sel->picklist() )
    {
      bool iType;
      const char * statusIndicator = computeStatusIndicator( pi, sel, &iType );
      if ( ( installed_only && !iType ) || ( notinst_only && iType) )
        continue;

      const std::string & piRepoName( pi.repoInfo().name() );
      if ( repofilter && piRepoName == "@System" )
        continue;

      // NOTE: 'Is Base' is available in the installed object only.
      tbl << ( TableRow()
          << statusIndicator
          << piRepoName
          << pi.name()
          << pi.summary()	// full name (bnc #589333)
          << pi.edition()
          << pi.arch()
          << asYesNo( iType && sel->identicalInstalledObj( pi )->asKind<Product>()->isTargetDistribution() ) );
    }
  }

  tbl.sort(1); // Name

  if ( tbl.empty() )
    zypper.out().info(_("No products found.") );
  else
    // display the result, even if --quiet specified
    cout << tbl;
}
