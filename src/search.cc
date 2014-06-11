#include <iostream>

#include <zypp/ZYpp.h> // for zypp::ResPool::instance()

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
#include "utils/misc.h" // for kind_to_string_localized and string_patch_status

#include "search.h"

using namespace zypp;
using namespace std;

extern ZYpp::Ptr God;

FillSearchTableSolvable::FillSearchTableSolvable(
    Table & table, zypp::TriBool inst_notinst)
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
{
  Zypper & zypper = *Zypper::instance();
  if (zypper.cOpts().find("repo") != zypper.cOpts().end())
  {
    list<RepoInfo> & repos = zypper.runtimeData().repos;
    for_(it, repos.begin(), repos.end())
      _repos.insert(it->alias());
  }

  TableHeader header;

  //
  // *** CAUTION: It's a mess, but adding/changing colums here requires
  //              adapting OutXML::searchResult !
  //
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
      << table::EditionStyleSetter( *_table, _("Version") )
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
      << table::EditionStyleSetter( *_table, _("Version") )
      // translators: package architecture (header)
      << _("Arch")
      // translators: package's repository (header)
      << _("Repository");
  }

  *_table << header;
}

bool FillSearchTableSolvable::addPicklistItem( const ui::Selectable::constPtr & sel, const PoolItem & pi ) const
{
  // --repo => we only want the repo resolvables, not @System (bnc #467106)
  if ( !_repos.empty() && _repos.find( pi->repoInfo().alias() ) == _repos.end() )
    return false;

  // hide patterns with user visible flag not set (bnc #538152)
  if ( pi->isKind<Pattern>() && ! pi->asKind<Pattern>()->userVisible() )
    return false;

  TableRow row;
  // compute status indicator:
  //   i  - exactly this version installed
  //   v  - installed, but in different version
  //      - not installed at all
  if ( pi->isSystem() )
  {
    // picklist: ==> not available
    if ( _inst_notinst == false )
      return false;	// show only not installed
    row << "i";
  }
  else
  {
    // picklist: ==> available, maybe identical installed too

    // only check for sel->installedEmpty() if NOT pseudo installed
    if ( !traits::isPseudoInstalled( pi->kind() ) && sel->installedEmpty() )
    {
      if ( _inst_notinst == true )
	return false;	// show only installed
      row << "";
    }
    else
    {
      bool identicalInstalledToo = ( traits::isPseudoInstalled( pi->kind() )
				   ? ( pi.isSatisfied() )
				   : ( sel->identicalInstalled( pi ) ) );
      if ( identicalInstalledToo )
      {
	if ( _inst_notinst == false )
	  return false;	// show only not installed
	row << "i";
      }
      else
      {
	if ( _inst_notinst == true )
	  return false;	// show only installed
	row << "v";
      }
    }
  }

  if ( _gopts.is_rug_compatible )
  {
    row
    << ( pi->isSystem()
       ? _("System Packages")
       : pi->repository().asUserString() )
    << ""
    << pi->name()
    << pi->edition().asString()
    << pi->arch().asString();
  }
  else
  {
    row
    << pi->name()
    << kind_to_string_localized( pi->kind(), 1 )
    << pi->edition().asString()
    << pi->arch().asString()
    << ( pi->isSystem()
       ? (string("(") + _("System Packages") + ")")
       : pi->repository().asUserString() );
  }

  *_table << row;
  return true;	// actually added a row
}

//
// PoolQuery iterator as argument provides information about matches
//
bool FillSearchTableSolvable::operator()( const zypp::PoolQuery::const_iterator & it ) const
{
  // all FillSearchTableSolvable::operator()( const zypp::PoolItem & pi )
  if ( ! operator()(*it) )
    return false;	// no row was added due to filter

  // after addPicklistItem( const ui::Selectable::constPtr & sel, const PoolItem & pi ) is
  // done, add the details about matches to last row
  TableRow & lastRow = _table->rows().back();

  // don't show details for patterns with user visible flag not set (bnc #538152)
  if (it->kind() == zypp::ResKind::pattern)
  {
    Pattern::constPtr ptrn = asKind<Pattern>(*it);
    if (ptrn && !ptrn->userVisible())
      return true;
  }

  if ( !it.matchesEmpty() )
  {
    for_( match, it.matchesBegin(), it.matchesEnd() )
    {
      std::string attrib( match->inSolvAttr().asString() );
      if ( str::startsWith( attrib, "solvable:" ) )	// strip 'solvable:' from attribute
	attrib.erase( 0, 9 );

      if ( match->inSolvAttr() == zypp::sat::SolvAttr::summary ||
           match->inSolvAttr() == zypp::sat::SolvAttr::description )
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

bool FillSearchTableSolvable::operator()( const zypp::PoolItem & pi ) const
{
  ui::Selectable::constPtr sel( ui::Selectable::get( pi ) );
  return addPicklistItem( sel, pi );
}

bool FillSearchTableSolvable::operator()( zypp::sat::Solvable solv ) const
{ return operator()( PoolItem( solv ) ); }

bool FillSearchTableSolvable::operator()( const zypp::ui::Selectable::constPtr & sel ) const
{
  bool ret = false;
  // picklist: available items list prepended by those installed items not identicalAvailable
  for_( it, sel->picklistBegin(), sel->picklistEnd() )
  {
    if ( addPicklistItem( sel, *it ) || !ret )
      ret = true;	// at least one row added
  }
  return ret;
}


FillSearchTableSelectable::FillSearchTableSelectable(
    Table & table, zypp::TriBool installed_only)
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , inst_notinst(installed_only)
{
  Zypper & zypper = *Zypper::instance();
  if (zypper.cOpts().find("repo") != zypper.cOpts().end())
  {
    list<RepoInfo> & repos = zypper.runtimeData().repos;
    for_(it, repos.begin(), repos.end())
      _repos.insert(it->alias());
  }

  TableHeader header;
  //
  // *** CAUTION: It's a mess, but adding/changing colums here requires
  //              adapting OutXML::searchResult !
  //
  // translators: S for installed Status
  header << _("S");
  header << _("Name");
  // translators: package summary (header)
  header << _("Summary");
  header << _("Type");
  *_table << header;
}

bool FillSearchTableSelectable::operator()(const zypp::ui::Selectable::constPtr & s) const
{
  // hide patterns with user visible flag not set (bnc #538152)
  if (s->kind() == zypp::ResKind::pattern)
  {
    Pattern::constPtr ptrn = s->candidateAsKind<Pattern>();
    if (ptrn && !ptrn->userVisible())
      return true;
  }

  TableRow row;

  // whether to show the solvable as 'installed'
  bool installed = false;

  if (zypp::traits::isPseudoInstalled(s->kind()))
    installed = s->theObj().isSatisfied();
  // check for installed counterpart in one of specified repos (bnc #467106)
  else if (!_repos.empty())
  {
    for_(ait, s->availableBegin(), s->availableEnd())
      for_(iit, s->installedBegin(), s->installedEnd())
        if (identical(*ait, *iit) &&
            _repos.find(ait->resolvable()->repoInfo().alias()) != _repos.end())
        {
          installed = true;
          break;
        }
  }
  // if no --repo is specified, we don't care where does the installed package
  // come from
  else
    installed = !s->installedEmpty();


  if (s->kind() != zypp::ResKind::srcpackage)
  {
    if (installed)
    {
      // not-installed only
      if (inst_notinst == false)
        return true;
      row << "i";
    }
    // this happens if the solvable has installed objects, but no counterpart
    // of them in specified repos
    else if (s->hasInstalledObj())
    {
      // not-installed only
      if (inst_notinst == true)
        return true;
      row << "v";
    }
    else
    {
      // installed only
      if (inst_notinst == true)
        return true;
      row << "";
    }
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


FillPatchesTable::FillPatchesTable( Table & table, zypp::TriBool inst_notinst )
  : _table( &table )
  , _gopts(Zypper::instance()->globalOpts())
  , _inst_notinst(inst_notinst)
{
  TableHeader header;

  header
    << (_gopts.is_rug_compatible ? _("Catalog") : _("Repository"))
    << _("Name")
    << _("Category")
    << _("Severity")
    << _("Status");

  *_table << header;
}

bool FillPatchesTable::operator()(const zypp::PoolItem & pi) const
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
    << pi->repository().asUserString()
    << pi->name()
    << patch->category()
    << patch->severity()
    << string_patch_status(pi);

  *_table << row;

  return true;
}


/*
string selectable_search_repo_str(const ui::Selectable & s)
{
  string repostr;

  // show available objects
  for_(it, s.availableBegin(), s.availableEnd())
  {
    if (repostr.empty())
      repostr = (*it)->repository().info().name();
    else if (repostr != (*it)->repository().info().name())
    {
      repostr = _("(multiple)");
      return repostr;
    }
  }

  return repostr;
}
*/

static string string_weak_status(const ResStatus & rs)
{
  if (rs.isRecommended())
    return _("Recommended");
  if (rs.isSuggested())
    return _("Suggested");
  return "";
}


void list_patches(Zypper & zypper)
{
  MIL
    << "Pool contains " << God->pool().size()
    << " items. Checking whether available patches are needed." << std::endl;

  Table tbl;

  FillPatchesTable callback(tbl);
  invokeOnEach(
    God->pool().byKindBegin(ResKind::patch),
    God->pool().byKindEnd(ResKind::patch),
    callback);

  tbl.sort (1);                 // Name

  if (tbl.empty())
    zypper.out().info(_("No needed patches found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

static void list_patterns_xml(Zypper & zypper)
{
  cout << "<pattern-list>" << endl;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  for_( it, God->pool().byKindBegin<Pattern>(), God->pool().byKindEnd<Pattern>() )
  {
    bool isInstalled = it->status().isInstalled();
    if ( isInstalled && notinst_only && !installed_only )
      continue;
    if ( !isInstalled && installed_only && !notinst_only )
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(it->resolvable());
    cout << asXML(*pattern, isInstalled) << endl;
  }

  cout << "</pattern-list>" << endl;
}

static void list_pattern_table(Zypper & zypper)
{
  MIL << "Going to list patterns." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  th << _("Name") << table::EditionStyleSetter( tbl, _("Version") );
  if (!zypper.globalOpts().is_rug_compatible)
    th << _("Repository") << _("Dependency");
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  for_( it, God->pool().byKindBegin<Pattern>(), God->pool().byKindEnd<Pattern>() )
  {
    bool isInstalled = it->status().isInstalled();
    if ( isInstalled && notinst_only && !installed_only )
      continue;
    else if ( !isInstalled && installed_only && !notinst_only )
      continue;

    Pattern::constPtr pattern = asKind<Pattern>(it->resolvable());
    // hide patterns with user visible flag not set (bnc #538152)
    if (!pattern->userVisible())
      continue;

    TableRow tr;
    tr << (isInstalled ? "i" : "");
    tr << pattern->name () << pattern->edition().asString();
    if (!zypper.globalOpts().is_rug_compatible)
    {
      tr << pattern->repoInfo().name();
      tr << string_weak_status(it->status ());
    }
    tbl << tr;
  }
  tbl.sort(1); // Name

  if (tbl.empty())
    zypper.out().info(_("No patterns found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_patterns(Zypper & zypper)
{
  if (zypper.out().type() == Out::TYPE_XML)
    list_patterns_xml(zypper);
  else
    list_pattern_table(zypper);
}

void list_packages(Zypper & zypper)
{
  MIL << "Going to list packages." << std::endl;
  Table tbl;

  const auto & copts( zypper.cOpts() );
  bool installed_only = copts.count("installed-only");
  bool uninstalled_only = copts.count("uninstalled-only");
  bool showInstalled = installed_only || !uninstalled_only;
  bool showUninstalled = uninstalled_only || !installed_only;

  bool orphaned = copts.count("orphaned");
  bool suggested = copts.count("suggested");
  bool recommended = copts.count("recommended");
  bool unneeded = copts.count("unneeded");
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
      zypp::PoolItem pi = *it;
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

      TableRow row;
      if ( s->hasInstalledObj() )
      {
	row << ( pi.status().isInstalled() || s->identicalInstalled( pi ) ? "i" : "v" );
      }
      else
      {
	row << "";
      }
      row << (zypper.globalOpts().is_rug_compatible ? "" : pi->repository().info().name())
          << pi->name()
          << pi->edition().asString()
          << pi->arch().asString();
      tbl << row;
    }
  }

  if (tbl.empty())
    zypper.out().info(_("No packages found."));
  else
  {
    // display the result, even if --quiet specified
    TableHeader th;
    // translators: S for installed Status
    th << _("S");
    if (zypper.globalOpts().is_rug_compatible)
      // translators: Bundle is a term used in rug. See rug for how to translate it.
    th << _("Bundle");
    else
      th << _("Repository");
    th << _("Name") << table::EditionStyleSetter( tbl, _("Version") ) << _("Arch");
    tbl << th;

    if (zypper.cOpts().count("sort-by-repo") || zypper.cOpts().count("sort-by-catalog"))
      tbl.sort(1); // Repo
    else
      tbl.sort(2); // Name

    cout << tbl;
  }
}

static void list_products_xml(Zypper & zypper)
{
  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  cout << "<product-list>" << endl;
  ResPool::byKind_iterator
    it = God->pool().byKindBegin(ResKind::product),
    e  = God->pool().byKindEnd(ResKind::product);
  for (; it != e; ++it )
  {
    if (it->status().isInstalled() && notinst_only)
      continue;
    else if (!it->status().isInstalled() && installed_only)
      continue;

    Product::constPtr product = asKind<Product>(it->resolvable());
    cout << asXML(*product, it->status().isInstalled()) << endl;
  }
  cout << "</product-list>" << endl;
}

// common product_table_row data
static void add_product_table_row( Zypper & zypper, TableRow & tr,  const Product::constPtr & product )
{
  // repository
  if (!zypper.globalOpts().is_rug_compatible)
    tr << product->repoInfo().name();
  // internal (unix) name
  tr << product->name ();
  // full name (bnc #589333)
  tr << product->summary();
  // version
  tr << product->edition().asString();
  if (zypper.globalOpts().is_rug_compatible)
    // rug 'Category'
    tr << (product->isTargetDistribution() ? "base" : "");
  else
  {
    // architecture
    tr << product->arch().asString();
    // is base
    tr << (product->isTargetDistribution() ? _("Yes") : _("No"));
  }
}

static void list_product_table(Zypper & zypper)
{
  MIL << "Going to list products." << std::endl;

  Table tbl;
  TableHeader th;

  // translators: S for installed Status
  th << _("S");
  if (!zypper.globalOpts().is_rug_compatible)
    th << _("Repository");
  // translators: used in products. Internal Name is the unix name of the
  // product whereas simply Name is the official full name of the product.
  th << _("Internal Name");
  th << _("Name");
  th << table::EditionStyleSetter( tbl, _("Version") );
  if (zypper.globalOpts().is_rug_compatible)
     // translators: product category (base/addon), the rug term
     th << _("Category");
  else
  {
    th << _("Arch");
    th << _("Is Base");
  }
  tbl << th;

  bool installed_only = zypper.cOpts().count("installed-only");
  bool notinst_only = zypper.cOpts().count("uninstalled-only");

  ResPoolProxy::const_iterator
      it = God->pool().proxy().byKindBegin(ResKind::product),
      e  = God->pool().proxy().byKindEnd(ResKind::product);
  for (; it != e; ++it )
  {
    ui::Selectable::constPtr s = *it;

    // get the first installed object
    PoolItem installed;
    if (!s->installedEmpty())
      installed = s->installedObj();

    bool missedInstalled( installed ); // if no available hits, we need to print it

    // show available objects
    for_(it, s->availableBegin(), s->availableEnd())
    {
      Product::constPtr product = asKind<Product>(it->resolvable());
      TableRow tr;
      zypp::PoolItem pi = *it;

      if (installed)
      {
        if (identical(installed, pi))
        {
          if (notinst_only || !missedInstalled)
            continue;
          tr << "i";
          // this is needed, other isTargetDistribution would not return
          // true for the installed base product
          product = asKind<Product>(installed);
          missedInstalled = false;
	  // bnc#841473: Downside of reporting the installed product (repo: @System)
	  // instead of the available one (repo the product originated from) is that
	  // you see multiple identical '@System' entries if multiple repos contain
	  // the same product. Thus don't report again, if !missedInstalled.
        }
        else
        {
          if (installed_only)
            continue;
          tr << "v";
        }
      }
      else
      {
        if (installed_only)
          continue;
        tr << "";
      }
      add_product_table_row( zypper, tr, product );
      tbl << tr;
    }

    if ( missedInstalled ) // no available hit, we need to print it
    {
      // show installed product in ablence of an available one:
      if (notinst_only)
        continue;

      TableRow tr;
      tr << "i";
      add_product_table_row( zypper, tr, installed->asKind<Product>() );
      tbl << tr;
    }
  }
  tbl.sort(1); // Name

  if (tbl.empty())
    zypper.out().info(_("No products found."));
  else
    // display the result, even if --quiet specified
    cout << tbl;
}

void list_products(Zypper & zypper)
{
  if (zypper.out().type() == Out::TYPE_XML)
    list_products_xml(zypper);
  else
    list_product_table(zypper);
}

// list_what_provides() isn't called any longer, ZypperCommand::WHAT_PROVIDES_e is
// replaced by Zypper::SEARCH_e with appropriate options (see Zypper.cc, line 919)
void list_what_provides(Zypper & zypper, const string & str)
{
  Capability cap(Capability::guessPackageSpec(str));
  sat::WhatProvides q(cap);

  // is there a provider for the requested capability?
  if (q.empty())
  {
    zypper.out().info(str::form(_("No providers of '%s' found."), str.c_str()));
    return;
  }

  // Ugly task of sorting the query (would be cool if table would do this):
  // 1st group by name
  std::map<IdString, std::vector<sat::Solvable>> res;
  for_( it, q.solvableBegin(), q.solvableEnd() )
  {
    res[(*it).ident()].push_back( *it );
  }
  // 2nd follow picklist (available items list prepended by those installed items not identicalAvailable)
  Table t;
  FillSearchTableSolvable fsts(t);
  for_( nameit, res.begin(), res.end() )
  {
    const ui::Selectable::Ptr sel( ui::Selectable::get( nameit->first ) );
    // replace installed by identical availabe if exists
    std::set<PoolItem> piset;
    for_( solvit, nameit->second.begin(), nameit->second.end() )
    {
      PoolItem pi( *solvit );
      if ( pi->isSystem() )
      {
	PoolItem identical( sel->identicalAvailableObj( pi ) );
	piset.insert( identical ? identical : pi );
      }
      else
	piset.insert( pi );
    }
    // follow picklist and print the ones we found
    for_( it, sel->picklistBegin(), sel->picklistEnd() )
    {
      if ( piset.find( *it ) != piset.end() )
	fsts.addPicklistItem( sel, *it );
    }
  }
  cout << t;
/*

  invokeOnEach(q.selectableBegin(), q.selectableEnd(), FillSearchTableSolvable(t) );
  cout << t;
  {
    zypp::ui::SelectableTraits::AvailableItemSet res( q.poolItemBegin(), q.poolItemEnd() );

    Table t;
    invokeOnEach(res.begin(), res.end(), FillSearchTableSolvable(t) );
    cout << t;
  }*/
}

// Local Variables:
// c-basic-offset: 2
// End:
