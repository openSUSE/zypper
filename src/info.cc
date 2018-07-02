/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/Algorithm.h>
#include <zypp/ZYpp.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>
#include <zypp/Pattern.h>
#include <zypp/Product.h>
#include <zypp/PoolQuery.h>

#include "Zypper.h"
#include "main.h"
#include "Table.h"
#include "utils/misc.h"
#include "utils/text.h"
#include "search.h"
#include "update.h"

#include "info.h"

extern ZYpp::Ptr God;

void printPkgInfo( Zypper & zypper, const ui::Selectable & s );
void printPatchInfo( Zypper & zypper, const ui::Selectable & s );
void printPatternInfo( Zypper & zypper, const ui::Selectable & s );
void printProductInfo( Zypper & zypper, const ui::Selectable & s );
void printSrcPackageInfo( Zypper & zypper, const ui::Selectable & s );
void printDefaultInfo( Zypper & zypper, const ui::Selectable & s );

///////////////////////////////////////////////////////////////////
namespace
{
  inline const std::vector<Dep> & cliSupportedDepTypes()
  {
    static const std::vector<Dep> _deps = {
      Dep::PROVIDES,
      Dep::REQUIRES,
      Dep::CONFLICTS,
      Dep::OBSOLETES,
      Dep::RECOMMENDS,
      Dep::SUGGESTS
    };
    return _deps;
  }

  std::string joinWords( std::string lhs, const std::string & rhs )
  {
    if ( ! rhs.empty() )
    {
      if ( ! lhs.empty() )
	lhs += " ";
      lhs += rhs;
    }
    return lhs;
  }

  inline void printCommonData( const PoolItem & pi_r, PropertyTable & tab_r )
  {
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Repository"),		pi_r.repository().asUserString() );
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Name"),		pi_r.name() );
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Version"),		pi_r.edition().asString() );
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Arch"),		pi_r.arch().asString() );
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Vendor"),		pi_r.vendor() );
  }

  inline void printSummaryDescDeps( const PoolItem & pi_r, PropertyTable & tab_r )
  {
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Summary"),		pi_r.summary() );
    // translators: property name; short; used like "Name: value"
    tab_r.addDetail( _("Description"),	::printRichText( pi_r.description() ) );

    // dependencies according to cli options and kind
    const auto & cOpts( Zypper::instance().cOpts() );
    bool isPatch = pi_r.isKind<Patch>();

    for ( const auto & dep : {
      Dep::PROVIDES,
      Dep::REQUIRES,
      Dep::CONFLICTS,
      Dep::OBSOLETES,
      Dep::RECOMMENDS,
      Dep::SUGGESTS,
      Dep::SUPPLEMENTS
    } )
    {
      if ( ( isPatch && ( dep == Dep::PROVIDES || dep == Dep::CONFLICTS ) )
	|| ( cOpts.count( dep.asString() ) ) )			// dep.asString - CLI option name
	tab_r.lst( dep.asUserString(), pi_r.dep( dep ) );	// dep.asUserString - i18n translation
    }
  }

  inline std::string propertyInstalled( const PoolItem & installedObj_r )
  {
    std::string ret( asYesNo( bool(installedObj_r) ) );
    if ( installedObj_r && installedObj_r.identIsAutoInstalled() )
    {
      ret += " (";
      ret += _("automatically");
      ret += ")";
    }
    return ret;
  }

  inline const char * lockStatusTag( const ui::Selectable & sel_r )
  { return ::lockStatusTag( sel_r.installedEmpty() ? "" : "i", sel_r.locked(), sel_r.identIsAutoInstalled() ); }

} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class KNSplit
  /// \brief Use the right kind and name.
  /// Prefer \ref ResKind explicitly specified in \a ident_r,
  /// else use the provided \a defaultKind_r.
  ///////////////////////////////////////////////////////////////////
  struct KNSplit
  {
    KNSplit( const std::string & ident_r, const ResKind & defaultKind_r = ResKind::nokind )
    : _kind( ResKind::explicitBuiltin( ident_r ) )
    {
      if ( ! _kind )
      {
	if ( defaultKind_r )
	  _kind = defaultKind_r;
	_name = ident_r;
      }
      else
      {
	// strip kind spec from name; ':' after kind is asserted
	_name = ident_r.substr( _kind.size()+1 );
      }
    }

    ResKind     _kind;
    std::string _name;
  };

  /** \relates KNSplit Stream output */
  inline std::ostream & operator<<( std::ostream & str, const KNSplit & obj )
  { return str << "[{" << obj._kind << "}{" << obj._name << "}]"; }

} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace {
  // PoolQuery does no COW :( - build and return the basic query
  inline PoolQuery printInfo_BasicQuery( Zypper & zypper )
  {
    PoolQuery baseQ;
    if ( zypper.cOpts().count("match-substrings") )
    { baseQ.setMatchSubstring(); }
    else
    { baseQ.setMatchGlob(); }	// is Exact if no glob chars included in name
    if ( zypper.cOpts().count("repo") )
    {
      for ( const RepoInfo & repo : zypper.runtimeData().repos  )
      { baseQ.addRepo( repo.alias() ); }
    }
    return baseQ;
  }

  void logOtherKindMatches( const PoolQuery & q_r, const std::string & name_r )
  {
    std::map<ResKind,DefaultIntegral<unsigned,0U>> count;
    for_( it, q_r.selectableBegin(), q_r.selectableEnd() )
    { ++count[(*it)->kind()]; }
    for ( const auto & pair : count )
    {
      cout << str::Format(PL_("There would be %1% match for '%2%'."
			     ,"There would be %1% matches for '%2%'."
			     ,pair.second))
			 % pair.second
			 % (pair.first.asString()+":"+name_r)
	   << endl;
    }
  }
} // namespace
///////////////////////////////////////////////////////////////////

void printInfo( Zypper & zypper, ResKindSet kinds_r )
{
  zypper.out().gap();

  for ( const std::string & rawarg : zypper.arguments() )
  {
    // Use the right kind!
    KNSplit kn( rawarg );

    PoolQuery q( printInfo_BasicQuery( zypper ) );
    q.addAttribute( sat::SolvAttr::name, kn._name );

    bool fallBackToAny = false;
    if ( kn._kind )
    {
      q.addKind( kn._kind );			// explicit kind in arg
    }
    else if ( !kinds_r.empty() )
    {
      for ( const auto & kind : kinds_r )	// wanted kinds via -t
	q.addKind( kind );
    }
    else
    {
      q.addKind( ResKind::package );
      fallBackToAny = true;			// Prefer packages, but fall back to any
    }

    if ( q.empty() )
    {
      ResKind oneKind( kn._kind );
      if ( !oneKind )
      {
	if ( !kinds_r.empty() && kinds_r.size() == 1 )
	  oneKind = *kinds_r.begin();
	else
	  oneKind = ResKind::package;
      }
      // TranslatorExplanation E.g. "package 'zypper' not found."
      cout << "\n" << str::Format(_("%s '%s' not found.")) % kind_to_string_localized( oneKind, 1 ) % rawarg << endl;

      // hint to matches of different kind (preferPackages looked for any)
      PoolQuery h( printInfo_BasicQuery( zypper ) );
      h.addAttribute( sat::SolvAttr::name, kn._name );

      if ( h.empty() )
	continue;
      else if ( !fallBackToAny )
      {
	logOtherKindMatches( h, kn._name );
	continue;
      }
      else
	q = h;
    }

    for_( it, q.selectableBegin(), q.selectableEnd() )
    {
      const ui::Selectable & sel( *(*it) );

      if ( zypper.out().type() != Out::TYPE_XML )
      {
	// TranslatorExplanation E.g. "Information for package zypper:"
	std::string info = str::Format(_("Information for %s %s:"))
				 % kind_to_string_localized( sel.kind(), 1 )
				 % sel.name();

	cout << endl << info << endl;
	cout << std::string( mbs_width(info), '-' ) << endl;
      }

      if      ( sel.kind() == ResKind::package )	{ printPkgInfo( zypper, sel ); }
      else if ( sel.kind() == ResKind::patch )		{ printPatchInfo( zypper, sel ); }
      else if ( sel.kind() == ResKind::pattern )	{ printPatternInfo( zypper, sel ); }
      else if ( sel.kind() == ResKind::product )	{ printProductInfo( zypper, sel ); }
      else if ( sel.kind() == ResKind::srcpackage)	{ printSrcPackageInfo( zypper, sel ); }
      else 						{ printDefaultInfo( zypper, sel ); }
    }
  }
}

/** Information for kinds which don't have a extra needs... */
void printDefaultInfo( Zypper & zypper, const ui::Selectable & s )
{
  PoolItem installed( s.installedObj() );
  PoolItem updateCand( s.updateCandidateObj() );
  // An updateCandidate is always better than any installed object.
  // If the best version is already installed try to look it up in
  // the repo it came from, otherwise use the installed one.
  PoolItem theone( updateCand );
  if ( !theone )
  {
    theone = s.identicalAvailableObj( installed );
    if ( !theone )
      theone = installed;
  }

  PropertyTable p;
  printCommonData( theone, p );

  printSummaryDescDeps( theone, p );
  zypper.out().info( str::Str() << p );
}

/**
 * Print package information.
 * <p>
 * Generates output like this:
<pre>
Repository     : @System
Name           : gvim
Version        : 6.4.6-19
Arch           : x86_64
Installed      : Yes
Status         : up-to-date
Installed Size : 2881221
Summary        : A GUI for Vi
Description    :
  Start /usr/X11R6/bin/gvim
  Copy and modify /usr/share/vim/current/gvimrc to ~/.gvimrc if needed.
</pre>
 *
 */
void printPkgInfo( Zypper & zypper, const ui::Selectable & s )
{
  PoolItem installed( s.installedObj() );
  PoolItem updateCand( s.updateCandidateObj() );
  // An updateCandidate is always better than any installed object.
  // If the best version is already installed try to look it up in
  // the repo it came from, otherwise use the installed one.
  PoolItem theone( updateCand );
  if ( !theone )
  {
    theone = s.identicalAvailableObj( installed );
    if ( !theone )
      theone = installed;
  }
  Package::constPtr package = theone->asKind<Package>();

  PropertyTable p;
  printCommonData( theone, p );

  // if running on SUSE Linux Enterprise, report unsupported packages
  if ( runningOnEnterprise() )
    p.add( _("Support Level"),	asUserString( theone->asKind<Package>()->vendorSupport() ) );

  // translators: property name; short; used like "Name: value"
  p.add( _("Installed Size"),	theone.installSize() );
  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),	propertyInstalled( installed ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Status") );
  if ( installed )
  {
    if ( updateCand )
       p.lastValue() = str::form(_("out-of-date (version %s installed)"), installed.edition().c_str() );
    else
      p.lastValue() = _("up-to-date");
  }
  else
      p.lastValue() = _("not installed");
  // translators: property name; short; used like "Name: value"
  p.add( _("Source package"),	package->sourcePkgLongName() );

  printSummaryDescDeps( theone, p );
  zypper.out().info( str::Str() << p );
}

/**
 * Print patch information.
 * <p>
 * Generates output like this:
 * <pre>
Name        : xv
Version     : 1448-0
Arch        : noarch
Status      : applied
Category    : recommended
Severity    : critical
Created On  : 5/31/2006 2:34:37 AM
Interactive : reboot | message | licence | restart
Summary     : XV can not grab in KDE
Description : XV can not grab in KDE
Provides    :
  xv = 1448-0
Requires:
  xv = 3.10a-1091.2
</pre>
 *
 */
void printPatchInfo( Zypper & zypper, const ui::Selectable & s )
{
  PoolItem theone( s.theObj() );
  Patch::constPtr patch = theone->asKind<Patch>();

  PropertyTable p;
  printCommonData( theone, p );

  // translators: property name; short; used like "Name: value"
  p.add( _("Status"),		i18nPatchStatus( theone )  );
  // translators: property name; short; used like "Name: value"
  p.add( _("Category"),		patchHighlightCategory( *patch ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Severity"),		patchHighlightSeverity( *patch ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Created On"),	patch->timestamp().asString() );
  // translators: property name; short; used like "Name: value"
  p.add( _("Interactive"),	patchInteractiveFlags( *patch ) );	// print interactive flags the same style as list-patches

  printSummaryDescDeps( theone, p );
  zypper.out().info( str::Str() << p );
}

/**
 * Print pattern information.
 * <p>
 * Generates output like this:
<pre>
Information for pattern sw_management:

Repository: factory
Name: sw_management
Version: 11.0-2
Arch: x86_64
Installed: Yes
Summary: Software Management
Description:
This pattern provides a graphical application and a command line tool for keeping your system up to date.
</pre>
 *
 */
void printPatternInfo( Zypper & zypper, const ui::Selectable & s )
{
  PoolItem theone( s.theObj() );
  Pattern::constPtr pattern = theone->asKind<Pattern>();

  PropertyTable p;
  printCommonData( theone, p );

  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),		propertyInstalled( s.installedObj() ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Visible to User"),		pattern->userVisible() );

  printSummaryDescDeps( theone, p );

  {	// show contents
    Pattern::ContentsSet collect;
    pattern->contentsSet( collect );

    bool showSuggests = zypper.cOpts().count( "suggests" );
    if ( collect.req.empty()
      && collect.rec.empty()
      && ( !showSuggests || collect.sug.empty() ) )
    {
      // translators: property name; short; used like "Name: value"
      p.addDetail( _("Contents"),	_("(empty)") );
    }
    else
    {
      Table t;
      t << ( TableHeader()
	  /* translators: Table column header */	<< _("S")
	  /* translators: Table column header */	<< _("Name")
	  /* translators: Table column header */	<< _("Type")
	  /* translators: Table column header */	<< _("Dependency") );

      for ( ui::Selectable::Ptr sel : collect.req.selectable() )
      {
	const ui::Selectable & s = *sel;
	t << ( TableRow() << lockStatusTag( s ) << s.name() << s.kind().asString() << _("Required") );
      }
      for ( ui::Selectable::Ptr sel : collect.rec.selectable() )
      {
	const ui::Selectable & s = *sel;
	t << ( TableRow() << lockStatusTag( s ) << s.name() << s.kind().asString() << _("Recommended") );
      }
      if ( showSuggests )
	for ( ui::Selectable::Ptr sel : collect.sug.selectable() )
	{
	  const ui::Selectable & s = *sel;
	  t << ( TableRow() << lockStatusTag( s ) << s.name() << s.kind().asString() << _("Suggested") );
	}

      std::map<std::string, unsigned> depPrio({{_("Required"),0}, {_("Recommended"),1}, {_("Suggested"),2}});
      t.sort( [&depPrio]( const TableRow & lhs, const TableRow & rhs ) -> bool {
	if ( lhs.columns()[3] != rhs.columns()[3] )
	  return depPrio[lhs.columns()[3]] < depPrio[rhs.columns()[3]];
	return  lhs.columns()[1] < rhs.columns()[1];
      } );

      // translators: property name; short; used like "Name: value"
      p.addDetail( _("Contents"),	str::Str() << t );
    }
  }
  zypper.out().info( str::Str() << p );
}

/**
 * Print product information.
 * <p>
 * Generates output like this:
<pre>
Information for product openSUSE-factory:

Repository: factory
Name: openSUSE-factory
Version: 11.0
Arch: x86_64
Category: base
Installed: No
Summary: openSUSE FACTORY 11.0
Description:
</pre>
 *
 */
void printProductInfo( Zypper & zypper, const ui::Selectable & s )
{
  if ( zypper.out().type() == Out::TYPE_XML )
  {
    PoolItem theone( s.theObj() );
    Product::constPtr product = theone->asKind<Product>();
    cout << asXML( *product, theone.status().isInstalled() ) << endl;
    return;
  }

  PoolItem installed( s.installedObj() );
  PoolItem updateCand( s.updateCandidateObj() );
  // An updateCandidate is always better than any installed object.
  // If the best version is already installed try to look it up in
  // the repo it came from, otherwise use the installed one.
  PoolItem theone( updateCand );
  if ( !theone )
  {
    theone = s.identicalAvailableObj( installed );
    if ( !theone )
      theone = installed;
  }

  PropertyTable p;
  printCommonData( theone, p );

  // Seems to be a feature:
  // commit#b40f49ae: printProductInfo: if installed, get info from installedObj
  Product::constPtr product = ( installed ? installed : theone )->asKind<Product>();

  {
    Date eol;
    if ( product->hasEndOfLife( eol ) )
    {
      // translators: property name; short; used like "Name: value"
      p.add( _("End of Support") );
      if ( eol )
	p.lastValue() = eol.printDate();
      else
      {
	p.lastValue() = _("unknown");
	if ( str::startsWithCI( product->vendor(), "suse" ) )
	{
	  p.lastValue() += "; ";
	  // translators: %1% is an URL or Path pointing to some document
	  p.lastValue() += str::Format(_("See %1%")) % "https://www.suse.com/lifecycle";
	}
      }
    }
  }
  // translators: property name; short; used like "Name: value"
  p.add( _("Flavor"),			joinWords( product->flavor(), product->registerFlavor() ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Is Base"),			bool(product->isTargetDistribution()) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),		propertyInstalled( installed ) );
  {
    // translators: property name; short; used like "Name: value"
    p.add( _("Status") );
    if ( installed )
    {
      if ( updateCand )
	p.lastValue() = str::form(_("out-of-date (version %s installed)"), installed.edition().c_str() );
      else
	p.lastValue() = _("up-to-date");
    }
    else
      p.lastValue() = _("not installed");
  }
  {
    const std::vector<Repository::ContentIdentifier> & cis( product->updateContentIdentifier() );
    if ( cis.empty() )
    {
      // translators: property name; short; used like "Name: value"
      p.add( _("Update Repositories"),	PropertyTable::emptyListTag() );
    }
    else
    {
      std::vector<std::string> lines;
      std::string prefix( _("Content Id") ); prefix += ": ";
      std::string indent( prefix.size(), ' ' );
      for ( const auto & el : cis )
      {
	str::Str str;
	str << prefix << el;
	bool found = false;
	for_( it, sat::Pool::instance().reposBegin(), sat::Pool::instance().reposEnd() )
	{
	  if ( (*it).hasContentIdentifier( el ) )
	  {
	    found = true;
	    str << endl << indent << _("Provided by enabled repository")   << ": " << (*it).name();
	  }
	}
	if ( ! found )
	{
	  str << endl << indent << ( ColorContext::MSG_WARNING << _("Not provided by any enabled repository") );
	}
	lines.push_back( str );
      }
      // translators: property name; short; used like "Name: value"
      p.lst( _("Update Repositories"),	lines, /*forceDetails_r*/true );
    }
  }
  {
    // translators: property name; short; used like "Name: value"
    p.add( _("CPE Name") );
    const CpeId & cpe( product->cpeId() );
    if ( cpe )
      p.lastValue() = cpe.asString();
    else if ( CpeId::NoThrowType::lastMalformed.empty() )
      p.lastValue() = _("undefined");
    else
      p.lastValue() = ( ColorContext::MSG_ERROR <<  _("invalid CPE Name") << ": " << CpeId::NoThrowType::lastMalformed ).str();
  }
  // translators: property name; short; used like "Name: value"
  p.add( _("Short Name"),		product->shortName() );

  printSummaryDescDeps( theone, p );
  zypper.out().info( str::Str() << p );
}

/**
 * Print source package information.
 */
///////////////////////////////////////////////////////////////////
namespace
{
  typedef std::pair<IdString,Edition>	BinPkg;
  typedef std::set<BinPkg>		BinPkgs;

  // TODO for libzypp: find packages built from 'theone'...
  // i.e. SolvAttr::sourcename/sourceevr mathes 'theone' name and edition
  BinPkgs findBinPkgsBuiltFromSrc( const PoolItem & theone_r )
  {
    BinPkgs ret;

    sat::LookupAttr query( sat::SolvAttr::sourcename );
    for_( it, query.begin(), query.end() )
    {
      if ( it.id() )			// found Solvable with a SolvAttr::sourcename attribute
      {
	// empty SolvAttr::sourcename means 'same as package'
	IdString srcname( it.id() == sat::detail::emptyId ? it.inSolvable().ident().id() : it.id() );
	if ( theone_r.ident() == srcname )	// name Solvable was built from matches, now check Edition...
	{
	  // empty SolvAttr::sourceevr means 'same as package'  (id-based comparisons are fine)
	  IdString srcevr( it.inSolvable().lookupIdAttribute( sat::SolvAttr::sourceevr ) );
	  if ( srcevr.empty() ) srcevr = IdString( it.inSolvable().edition().id() );
	  if ( theone_r.edition().id() == srcevr.id() )
	  {
	    //std::cerr << it.inSolvable() << " - " << it.id() << " (" << it.inSolvable().ident().id() << ") | " << srcname << endl;
	    ret.insert( { it.inSolvable().ident(), it.inSolvable().edition() } );
	  }
	}
      }
    }
    return ret;
  }

  // TODO Use a common status tag computation here, in search.cc, ...
  const char * BinPkgStatus( const BinPkg & binPkg_r )
  {
    ui::Selectable::Ptr sel( ui::Selectable::get( binPkg_r.first ) );
    if ( sel->installedEmpty() )
      return "";
    for ( const PoolItem & pi : sel->installed() )
      if ( pi.edition() == binPkg_r.second )
	return "i";
    return "v";
  }

} // namespace
///////////////////////////////////////////////////////////////////

void printSrcPackageInfo( Zypper & zypper, const ui::Selectable & s )
{
  PoolItem theone( s.theObj() );
  SrcPackage::constPtr srcPackage = theone->asKind<SrcPackage>();

  PropertyTable p;
  printCommonData( theone, p );
  printSummaryDescDeps( theone, p );

  ///////////////////////////////////////////////////////////////////
  BinPkgs builtFrom( findBinPkgsBuiltFromSrc( theone ) );
  if ( ! builtFrom.empty() )
  {
    Table t;
    t << ( TableHeader()
	/* translators: Table column header */	<< _("S")
	/* translators: Table column header */	<< _("Name")
	/* translators: Table column header */	<< _("Version") );

    for ( auto binPkg : builtFrom )
    { t << ( TableRow() << BinPkgStatus( binPkg ) << binPkg.first << binPkg.second ); }

    t.sort( 1 );

    // translators: property name; short; used like "Name: value"; is followed by a list of binary packages built from this source package
    p.addDetail( _("Builds binary package"),	str::Str() << t );
  }
  ///////////////////////////////////////////////////////////////////

  zypper.out().info( str::Str() << p );
}
