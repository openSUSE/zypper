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
	  << _("S")
	  // translators: name (general header)
	  << _("Name")
	  // translators: type (general header)
	  << _("Type")
	  // translators: package version (header)
	  << table::EditionStyleSetter( *_table, _("Version") )
	  // translators: package architecture (header)
	  << _("Arch")
	  // translators: package's repository (header)
	  << _("Repository") );
}

bool FillSearchTableSolvable::operator()( const PoolItem & pi_r ) const
{
  // --repo => we only want the repo resolvables, not @System (bnc #467106)
  if ( !_repos.empty() && !_repos.count( pi_r.repoInfo().alias() ) )
    return false;

  // hide patterns with user visible flag not set (bnc #538152)
  if ( pi_r->isKind<Pattern>() && ! pi_r->asKind<Pattern>()->userVisible() )
    return false;


  TableRow row;
  // compute status indicator:
  //   i  - exactly this version installed
  //   v  - installed, but in different version
  //      - not installed at all
  bool isLocked = pi_r.status().isLocked();
  ui::Selectable::Ptr sel( ui::Selectable::get( pi_r ) );

  if ( pi_r->isSystem() )
  {
    // pi_rcklist: ==> not available
    if ( _instNotinst == false || sel->identicalAvailable( pi_r ) )
      return false;	// show only not installed
    row << lockStatusTag( "i", isLocked, pi_r.identIsAutoInstalled() );
  }
  else
  {
    // picklist: ==> available, maybe identical installed too

    // only check for sel->installedEmpty() if NOT pseudo installed
    if ( !traits::isPseudoInstalled( pi_r->kind() ) && sel->installedEmpty() )
    {
      if ( _instNotinst == true )
	return false;	// show only installed
      row << lockStatusTag( "", isLocked );
    }
    else
    {
      bool identicalInstalledToo = ( traits::isPseudoInstalled( pi_r->kind() )
				   ? ( pi_r.isSatisfied() )
				   : ( sel->identicalInstalled( pi_r ) ) );
      if ( identicalInstalledToo )
      {
	if ( _instNotinst == false )
	  return false;	// show only not installed
	row << lockStatusTag( "i", isLocked, pi_r.identIsAutoInstalled() );
      }
      else
      {
	if ( _instNotinst == true )
	  return false;	// show only installed
	row << lockStatusTag( "v", isLocked );
      }
    }
  }

  row
    << pi_r->name()
    << kind_to_string_localized( pi_r->kind(), 1 )
    << pi_r->edition().asString()
    << pi_r->arch().asString()
    << ( pi_r->isSystem()
       ? (std::string("(") + _("System Packages") + ")")
       : pi_r->repository().asUserString() );

  row.userData( SolvableCSI(pi_r.satSolvable(), sel->picklistPos(pi_r)));

  *_table << row;

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
, inst_notinst( installed_only )
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
	  // translators: S for installed Status
	  << _("S")
	  << _("Name")
	  // translators: package summary (header)
	  << _("Summary")
	  << _("Type") );
}

bool FillSearchTableSelectable::operator()( const ui::Selectable::constPtr & s ) const
{
  // hide patterns with user visible flag not set (bnc #538152)
  if ( s->kind() == ResKind::pattern )
  {
    Pattern::constPtr ptrn = s->candidateAsKind<Pattern>();
    if ( ptrn && !ptrn->userVisible() )
      return true;
  }

  TableRow row;

  // whether to show the solvable as 'installed'
  bool installed = false;

  if ( traits::isPseudoInstalled( s->kind() ) )
    installed = s->theObj().isSatisfied();
  // check for installed counterpart in one of specified repos (bnc #467106)
  else if (!_repos.empty())
  {
    for_( ait, s->availableBegin(), s->availableEnd() )
      for_( iit, s->installedBegin(), s->installedEnd() )
        if ( identical( *ait, *iit ) && _repos.find( ait->repoInfo().alias() ) != _repos.end() )
        {
          installed = true;
          break;
        }
  }
  // if no --repo is specified, we don't care where does the installed package
  // come from
  else
    installed = !s->installedEmpty();

  bool isLocked = s->locked();
  if ( s->kind() != ResKind::srcpackage )
  {
    if ( installed )
    {
      // not-installed only
      if ( inst_notinst == false )
        return true;
      row << lockStatusTag( "i", isLocked, s->identIsAutoInstalled() );
    }
    // this happens if the solvable has installed objects, but no counterpart
    // of them in specified repos
    else if ( s->hasInstalledObj() )
    {
      // not-installed only
      if ( inst_notinst == true )
        return true;
      row << lockStatusTag( "v", isLocked );
    }
    else
    {
      // installed only
      if ( inst_notinst == true )
        return true;
      row << lockStatusTag( "", isLocked );
    }
  }
  else
  {
    // installed only
    if ( inst_notinst == true )
      return true;
    row << lockStatusTag( "", isLocked );
  }

  row << s->name();
  row << s->theObj()->summary();
  row << kind_to_string_localized( s->kind(), 1 );
  *_table << row;
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
  FillPatchesTable callback( tbl );
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

  for_( it, God->pool().byKindBegin<Pattern>(), God->pool().byKindEnd<Pattern>() )
  {
    bool isInstalled = it->status().isInstalled();
    if ( isInstalled && notinst_only && !installed_only )
      continue;
    if ( !isInstalled && installed_only && !notinst_only )
      continue;
    if ( repofilter && it->repository().info().name() == "@System" )
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(it->resolvable());
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
      << _("S")
      << _("Name")
      << table::EditionStyleSetter( tbl, _("Version") )
      << _("Repository")
      << _("Dependency") );

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only   = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  for( const PoolItem & pi : God->pool().byKind<Pattern>() )
  {
    bool isInstalled = pi.status().isInstalled();
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

    bool isLocked = pi.status().isLocked();
    tbl << ( TableRow()
	<< (isInstalled ? lockStatusTag( "i", isLocked, pi.identIsAutoInstalled() ) : lockStatusTag( "", isLocked ))
	<< pi.name()
	<< pi.edition()
	<< piRepoName
	<< string_weak_status(pi.status()) );
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

  const auto & pproxy( God->pool().proxy() );
  for_( it, pproxy.byKindBegin(ResKind::package), pproxy.byKindEnd(ResKind::package) )
  {
    ui::Selectable::constPtr s = *it;
    // filter on selectable level
    if ( s->hasInstalledObj() )
    {
      if ( ! showInstalled )
	continue;
    }
    else
    {
      if ( ! showUninstalled )
	continue;
    }

    for_( it, s->picklistBegin(), s->picklistEnd() )
    {
      PoolItem pi = *it;
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
	  PoolItem ipi( s->identicalInstalledObj( pi ) );
	  if ( !ipi || !checkStatus( ipi.status() ) )
	    if ( ! checkStatus( pi.status() ) )
	      continue;
	}
      }

      const std::string & piRepoName( pi->repository().info().name() );
      if ( repofilter && piRepoName == "@System" )
	continue;

      TableRow row;
      bool isLocked = pi.status().isLocked();
      if ( s->hasInstalledObj() )
      {
	row << ( pi.status().isInstalled() || s->identicalInstalled( pi ) ? lockStatusTag( "i", isLocked, pi.identIsAutoInstalled() ) : lockStatusTag( "v", isLocked ) );
      }
      else
      {
	row << lockStatusTag( "", isLocked );
      }
      row << piRepoName
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();
      tbl << row;
    }
  }

  if ( tbl.empty() )
    zypper.out().info(_("No packages found.") );
  else
  {
    // display the result, even if --quiet specified
    tbl << ( TableHeader()
	// translators: S for installed Status
	<< _("S")
	<< _("Repository")
	<< _("Name")
	<< table::EditionStyleSetter( tbl, _("Version") )
	<< _("Arch") );

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
  for_( it, God->pool().byKindBegin(ResKind::product), God->pool().byKindEnd(ResKind::product) )
  {
    if ( it->status().isInstalled() && notinst_only )
      continue;
    else if ( !it->status().isInstalled() && installed_only )
      continue;
    if ( repofilter && it->repository().info().name() == "@System" )
      continue;
    Product::constPtr product = asKind<Product>(it->resolvable());
    cout << asXML( *product, it->status().isInstalled(), fwdTags ) << endl;
  }
  cout << "</product-list>" << endl;
}

// common product_table_row data
static void add_product_table_row( Zypper & zypper, TableRow & tr,  const Product::constPtr & product, bool forceShowAsBaseProduct_r = false )
{
  // repository
  tr << product->repoInfo().name();
  // internal (unix) name
  tr << product->name ();
  // full name (bnc #589333)
  tr << product->summary();
  // version
  tr << product->edition().asString();
  // architecture
  tr << product->arch().asString();
  // is base
  tr << asYesNo( forceShowAsBaseProduct_r || product->isTargetDistribution() );
}

void list_product_table(Zypper & zypper , SolvableFilterMode mode_r)
{
  MIL << "Going to list products." << std::endl;

  Table tbl;

  tbl << ( TableHeader()
      // translators: S for installed Status
      << _("S")
      << _("Repository")
      // translators: used in products. Internal Name is the unix name of the
      // product whereas simply Name is the official full name of the product.
      << _("Internal Name")
      << _("Name")
      << table::EditionStyleSetter( tbl, _("Version") )
      << _("Arch")
      << _("Is Base") );

  bool repofilter =  InitRepoSettings::instance()._repoFilter.size() ;	// suppress @System if repo filter is on
  bool installed_only = mode_r == SolvableFilterMode::ShowOnlyInstalled;
  bool notinst_only = mode_r == SolvableFilterMode::ShowOnlyNotInstalled;

  for_( it, God->pool().proxy().byKindBegin(ResKind::product), God->pool().proxy().byKindEnd(ResKind::product) )
  {
    ui::Selectable::constPtr s = *it;

    // get the first installed object
    PoolItem installed;
    if ( !s->installedEmpty() )
      installed = s->installedObj();

    bool missedInstalled( installed ); // if no available hits, we need to print it

    // show available objects
    for_( it, s->availableBegin(), s->availableEnd() )
    {
      Product::constPtr product = asKind<Product>(it->resolvable());
      TableRow tr;
      PoolItem pi = *it;
      bool isLocked = pi.status().isLocked();
      bool forceShowAsBaseProduct = false;

      if ( installed )
      {
        if ( missedInstalled && identical( installed, pi ) )
        {
          if ( notinst_only )
            continue;
          tr << lockStatusTag( "i", isLocked, pi.identIsAutoInstalled() );
          // isTargetDistribution (i.e. is Base Product) needs to be taken from the installed item!
          forceShowAsBaseProduct = installed->asKind<Product>()->isTargetDistribution();
          missedInstalled = false;
	  // bnc#841473: Downside of reporting the installed product (repo: @System)
	  // instead of the available one (repo the product originated from) is that
	  // you see multiple identical '@System' entries if multiple repos contain
	  // the same product. Thus don't report again, if !missedInstalled.
        }
        else
        {
          if ( installed_only )
            continue;
          tr << lockStatusTag( "v", isLocked );
        }
      }
      else
      {
        if ( installed_only )
          continue;
        tr << lockStatusTag( "", isLocked );
      }
      add_product_table_row( zypper, tr, product, forceShowAsBaseProduct );
      tbl << tr;
    }

    if ( missedInstalled ) // no available hit, we need to print it
    {
      // show installed product in absence of an available one:
      if ( notinst_only || repofilter )
        continue;

      TableRow tr;
      bool isLocked = installed.status().isLocked();
      tr << lockStatusTag( "i", isLocked, installed.identIsAutoInstalled() );
      add_product_table_row( zypper, tr, installed->asKind<Product>() );
      tbl << tr;
    }
  }
  tbl.sort(1); // Name

  if ( tbl.empty() )
    zypper.out().info(_("No products found.") );
  else
    // display the result, even if --quiet specified
    cout << tbl;
}
