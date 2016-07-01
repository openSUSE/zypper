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
    const auto & cOpts( Zypper::instance()->cOpts() );
    bool isPatch = pi_r.isKind<Patch>();

    for ( const auto & dep : {
      Dep::PROVIDES,
      Dep::REQUIRES,
      Dep::CONFLICTS,
      Dep::OBSOLETES,
      Dep::RECOMMENDS,
      Dep::SUGGESTS
    } )
    {
      if ( ( isPatch && ( dep == Dep::PROVIDES || dep == Dep::CONFLICTS ) )
	|| ( cOpts.count( dep.asString() ) ) )			// dep.asString - CLI option name
	tab_r.lst( dep.asUserString(), pi_r.dep( dep ) );	// dep.asUserString - i18n translation
    }
  }
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

void printInfo( Zypper & zypper, const ResKind & kind_r )
{
  zypper.out().gap();

  for ( const std::string & rawarg : zypper.arguments() )
  {
    // Use the right kind!
    KNSplit kn( rawarg, kind_r );

    PoolQuery q;
    q.addKind( kn._kind );
    q.addAttribute( sat::SolvAttr::name, kn._name );

    if ( zypper.cOpts().count("match-substrings") )
    { q.setMatchSubstring(); }
    else
    { q.setMatchGlob(); }	// is Exact if no glob chars included in name

    if ( q.empty() )
    {
      // TranslatorExplanation E.g. "package 'zypper' not found."
      cout << "\n" << str::Format(_("%s '%s' not found.")) % kind_to_string_localized( kn._kind, 1 ) % rawarg << endl;
      {
	// hint to matches of different kind
	PoolQuery q;
	q.addAttribute( sat::SolvAttr::name, kn._name );
	if ( zypper.cOpts().count("match-substrings") )
	{ q.setMatchSubstring(); }
	else
	{ q.setMatchGlob(); }	// is Exact if no glob chars included in name
	if ( ! q.empty() )
	  logOtherKindMatches( q, kn._name );
      }
      continue;
    }

    for_( it, q.selectableBegin(), q.selectableEnd() )
    {
      if ( zypper.out().type() != Out::TYPE_XML )
      {
	// TranslatorExplanation E.g. "Information for package zypper:"
	std::string info = str::Format(_("Information for %s %s:"))
				 % kind_to_string_localized( kn._kind, 1 )
				 % (*it)->name();

	cout << endl << info << endl;
	cout << std::string( mbs_width(info), '-' ) << endl;
      }

      if ( kn._kind == ResKind::package )	{ printPkgInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::patch )	{ printPatchInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::pattern )	{ printPatternInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::product )	{ printProductInfo( zypper, *(*it) ); }
      else 					{ printDefaultInfo( zypper, *(*it) ); }
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

  PropertyTable p;
  printCommonData( theone, p );

  // if running on SUSE Linux Enterprise, report unsupported packages
  if ( runningOnEnterprise() )
    p.add( _("Support Level"),	asUserString( theone->asKind<Package>()->vendorSupport() ) );

  // translators: property name; short; used like "Name: value"
  p.add( _("Installed Size"),	theone.installSize() );
  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),	bool(installed) );
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

#if 0
  p.add( _("Reboot Required"),	patch->rebootSuggested() );
  p.add( _("Package Manager Restart Required"),	patch->restartSuggested() );

  Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
  if ( zypper.globalOpts().reboot_req_non_interactive )
    ignoreFlags |= Patch::Reboot;
  if ( zypper.cOpts().count("auto-agree-with-licenses") || zypper.cOpts().count("agree-to-third-party-licenses") )
    ignoreFlags |= Patch::License;
  p.add( _("Interactive"),	patch->interactiveWhenIgnoring( ignoreFlags ) );
#else
  // translators: property name; short; used like "Name: value"
  p.add( _("Interactive"),	patchInteractiveFlags( *patch ) );	// print interactive flags the same style as list-patches
#endif

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
  p.add( _("Installed"),		s.hasInstalledObj() );
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
	  /* translators: Table coumn header */	<< _("S")
	  /* translators: Table coumn header */	<< _("Name")
	  /* translators: Table coumn header */	<< _("Type")
	  /* translators: Table coumn header */	<< _("Dependency") );

      for ( ui::Selectable::Ptr sel : collect.req.selectable() )
      {
	const ui::Selectable & s = *sel;
	t << ( TableRow() << (s.installedEmpty() ? "" : "i") << s.name() << s.kind().asString() << _("Required") );
      }
      for ( ui::Selectable::Ptr sel : collect.rec.selectable() )
      {
	const ui::Selectable & s = *sel;
	t << ( TableRow() << (s.installedEmpty() ? "" : "i") << s.name() << s.kind().asString() << _("Recommended") );
      }
      if ( showSuggests )
	for ( ui::Selectable::Ptr sel : collect.sug.selectable() )
	{
	  const ui::Selectable & s = *sel;
	  t << ( TableRow() << (s.installedEmpty() ? "" : "i") << s.name() << s.kind().asString() << _("Suggested") );
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
    Date eol( product->endOfLife() );
    // translators: property name; short; used like "Name: value"
    p.add( _("End of Support"),		( eol ? eol.printDate() : _("undefined") ) );
  }
  // translators: property name; short; used like "Name: value"
  p.add( _("Flavor"),			joinWords( product->flavor(), product->registerFlavor() ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Is Base"),			bool(product->isTargetDistribution()) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),		bool(installed) );
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
