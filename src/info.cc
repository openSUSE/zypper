/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

// #include <zypp/base/LogTools.h>
#include <zypp/base/Algorithm.h>
#include <zypp/ZYpp.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>
#include <zypp/Pattern.h>
#include <zypp/Product.h>
#include <zypp/PoolQuery.h>

#include "Zypper.h"
#include "main.h"
//#include "misc.h"
#include "Table.h"
#include "utils/misc.h" // for kind_to_string_localized and string_patch_status
#include "utils/text.h"
#include "search.h"
#include "update.h"

#include "info.h"

using namespace std;
using namespace zypp;

extern ZYpp::Ptr God;

void printPkgInfo(Zypper & zypper, const zypp::ui::Selectable & s);
void printPatchInfo(Zypper & zypper, const zypp::ui::Selectable & s);
void printPatternInfo(Zypper & zypper, const zypp::ui::Selectable & s);
void printProductInfo(Zypper & zypper, const zypp::ui::Selectable & s);

///////////////////////////////////////////////////////////////////
namespace
{
  inline std::string asCliOption( Dep dep_r )
  { return dep_r.asString(); }

  inline std::string asInfoTag( Dep dep_r )
  { return dep_r.asUserString(); }

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

  inline void printDepList( const PoolItem & pi_r, Dep dep_r )
  {
    cout << asInfoTag( dep_r ) << ':' << endl;
    for ( auto && cap : pi_r->dep( dep_r ) )
    { cout << "  " << cap << endl; }
  }

  inline std::ostream & appendWord( std::ostream & str, const std::string & word_r )
  {
    if ( word_r.empty() )
      return str;
    return str << " " << word_r;
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
  std::ostream & operator<<( std::ostream & str, const KNSplit & obj )
  { return str << "[{" << obj._kind << "}{" << obj._name << "}]"; }

} // namespace
///////////////////////////////////////////////////////////////////

void printNVA( const PoolItem & pi_r )
{
  cout << _("Name: ") << pi_r.name() << endl;
  cout << _("Version: ") << pi_r.edition().asString() << endl;
  cout << _("Arch: ") << pi_r.arch().asString() << endl;
  cout << _("Vendor: ") << pi_r.vendor() << endl;
}

void printSummaryDesc( const PoolItem & pi_r )
{
  cout << _("Summary: ") << pi_r.summary() << endl;
  cout << _("Description: ") << endl;
  Zypper::instance()->out().printRichText( pi_r.description(), 2/*indented*/ );
}

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
      // print info
      // TranslatorExplanation E.g. "Information for package zypper:"

      if ( zypper.out().type() != Out::TYPE_XML )
      {
	string info = str::Format(_("Information for %s %s:"))
				 % kind_to_string_localized( kn._kind, 1 )
				 % (*it)->name();

	cout << endl << info << endl;
	cout << string( mbs_width(info), '-' ) << endl;
      }


      if ( kn._kind == ResKind::package )	{ printPkgInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::patch )	{ printPatchInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::pattern )	{ printPatternInfo( zypper, *(*it) ); }
      else if ( kn._kind == ResKind::product )	{ printProductInfo( zypper, *(*it) ); }
      else
      {
	// TranslatorExplanation %s = resolvable type (package, patch, pattern, etc - untranslated).
	zypper.out().info( str::Format(_("Info for type '%s' not implemented.")) % kn._kind );
      }
    }
  }
}


/**
 * Print package information.
 * <p>
 * Generates output like this:
<pre>
Repository: system
Name: gvim
Version: 6.4.6-19
Arch: x86_64
Installed: Yes
Status: up-to-date
Installed Size: 2881221
Summary: A GUI for Vi
Description: Start: /usr/X11R6/bin/gvim

Copy and modify /usr/share/vim/current/gvimrc to ~/.gvimrc if needed.
</pre>
 *
 */
void printPkgInfo(Zypper & zypper, const ui::Selectable & s)
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

  cout << _("Repository: ") << theone.repository().asUserString() << endl;

  printNVA( theone );

  // if running on SUSE Linux Enterprise, report unsupported packages
  if ( runningOnEnterprise() )
  {
    Package::constPtr pkg = asKind<Package>(theone.resolvable());
    cout << _("Support Level: ") << asUserString(pkg->vendorSupport()) << endl;
  }

  cout << _("Installed: ") << (installed ? _("Yes") : _("No")) << endl;

  cout << _("Status: ");
  if ( installed )
  {
    if ( updateCand )
    {
      cout << str::form(_("out-of-date (version %s installed)"), installed.edition().c_str()) << endl;
    }
    else
    {
      cout << _("up-to-date") << endl;
    }
  }
  else
    cout << _("not installed") << endl;

  cout << _("Installed Size: ") << theone.installSize() << endl;

  printSummaryDesc( theone );

  // Print dependency lists if CLI requests it
  for ( auto && dep : cliSupportedDepTypes() )
  { if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( theone, dep ); }
}

/**
 * Print patch information.
 * <p>
 * Generates output like this:
 * <pre>
Name: xv
Version: 1448-0
Arch: noarch
Status: Satisfied
Category: recommended
Created On: 5/31/2006 2:34:37 AM
Reboot Required: No
Package Manager Restart Required: No
Interactive: No
Summary: XV can not grab in KDE
Description: XV can not grab in KDE
Provides:
patch: xv = 1448-0

Requires:
atom: xv = 3.10a-1091.2
</pre>
 *
 */
void printPatchInfo(Zypper & zypper, const ui::Selectable & s )
{
  const PoolItem & pool_item = s.theObj();
  printNVA( pool_item );

  cout << _("Status: ") << string_patch_status(pool_item) << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << _("Category: ") << patch->category() << endl;
  cout << _("Severity: ") << patch->severity() << endl;
  cout << _("Created On: ") << patch->timestamp().asString() << endl;
  cout << _("Reboot Required: ") << (patch->rebootSuggested() ? _("Yes") : _("No")) << endl;
  cout << _("Package Manager Restart Required") << ": ";
  cout << (patch->restartSuggested() ? _("Yes") : _("No")) << endl;

  Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
  if (zypper.globalOpts().reboot_req_non_interactive)
    ignoreFlags |= Patch::Reboot;
  if ( zypper.cOpts().count("auto-agree-with-licenses") || zypper.cOpts().count("agree-to-third-party-licenses") )
    ignoreFlags |= Patch::License;

  cout << _("Interactive: ") << (patch->interactiveWhenIgnoring(ignoreFlags) ? _("Yes") : _("No")) << endl;

  printSummaryDesc( pool_item );

  // Print dependency lists if CLI requests it
  for ( auto && dep : cliSupportedDepTypes() )
  {
    switch ( dep.inSwitch() )
    {
      case Dep::PROVIDES_e:
      case Dep::CONFLICTS_e:
	printDepList( pool_item, dep );	// These dependency lists are always printed
	break;
      default:
	if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( pool_item, dep );
	break;
    }
  }
}

static string string_weak_status(const ResStatus & rs)
{
  if (rs.isRecommended())
    return _("Recommended");
  if (rs.isSuggested())
    return _("Suggested");
  return "";
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
void printPatternInfo(Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj();
  Pattern::constPtr pattern = asKind<Pattern>(pool_item.resolvable());

  if ( !pattern )
    return;

  cout << _("Repository: ") << pool_item.repository().asUserString() << endl;

  printNVA( pool_item );

  cout << _("Installed: ") << (s.hasInstalledObj() ? _("Yes") : _("No")) << endl;
  cout << _("Visible to User: ") << (pattern->userVisible() ? _("Yes") : _("No")) << endl;

  printSummaryDesc( pool_item );

  // Print dependency lists if CLI requests it
  for ( auto && dep : cliSupportedDepTypes() )
  { if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( pool_item, dep ); }

  // show contents
  Table t;
  TableHeader th;
  th << _("S") << _("Name") << _("Type") << _("Dependency");
  t << th;

  Pattern::Contents contents = pattern->contentsNoSuggests();	// (bnc#857671) don't include suggests as we can not deal with them .
  for_(sit, contents.selectableBegin(), contents.selectableEnd())
  {
    const ui::Selectable & s = **sit;
    TableRow tr;

    tr << (s.installedEmpty() ? "" : "i");
    tr << s.name() << s.kind().asString() << string_weak_status(s.theObj().status());

    t << tr;
  }

  cout << _("Contents") << ":";
  if (t.empty())
    cout << " " << _("(empty)") << endl;
  else
  {
    t.sort( 1 );
    cout << endl << endl << t;
  }
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
void printProductInfo(Zypper & zypper, const ui::Selectable & s)
{
  const PoolItem & pool_item = s.theObj(); // should be the only one

  if (zypper.out().type() == Out::TYPE_XML)
  {
    Product::constPtr pp = asKind<Product>(pool_item.resolvable());
    cout
      << asXML(*pp, pool_item.status().isInstalled())
      << endl;
  }
  else
  {
    cout << _("Repository: ") << pool_item.repository().asUserString() << endl;

    printNVA( pool_item );

    PoolItem installed;
    if (!s.installedEmpty())
      installed = s.installedObj();

    Product::constPtr product;
    if ( installed )
      product = asKind<Product>(installed);
    else
      product = asKind<Product>(pool_item.resolvable());

    cout << _("Flavor") << ":";
    appendWord( cout, product->flavor() );
    appendWord( cout, product->registerFlavor() );
    cout << endl;

    cout << _("Short Name") << ": " << product->shortName() << endl;

    cout << _("Installed")  << ": " << ( installed ? _("Yes") : _("No") )<< endl;

    cout << _("Is Base")   << ": " << (product->isTargetDistribution()  ? _("Yes") : _("No")) << endl;

    {
      Date eol( product->endOfLife() );
      cout << _("End of Support") << ": " << ( eol ? eol.printDate() : _("undefined") ) << endl;
    }
    {
      cout << _("CPE Name") << ": ";
      const CpeId & cpe( product->cpeId() );
      if ( cpe )
	cout << cpe << endl;
      else if ( CpeId::NoThrowType::lastMalformed.empty() )
	cout <<  _("undefined") << endl;
      else
	cout << ( ColorContext::MSG_ERROR <<  _("invalid CPE Name") << ": " << CpeId::NoThrowType::lastMalformed ) << endl;
    }
    {
      cout << _("Update Repositories");
      const std::vector<Repository::ContentIdentifier> & l( product->updateContentIdentifier() );
      if ( ! l.empty() )
      {
	cout << ": " << l.size() << endl;
	unsigned n = 0;
	for ( const auto & el : l )
	{
	  cout << "[" << ++n << "] " << _("Content Id")   << ": " << el << endl;
	  bool found = false;
	  for_( it, sat::Pool::instance().reposBegin(), sat::Pool::instance().reposEnd() )
	  {
	    if ( (*it).hasContentIdentifier( el ) )
	    {
	      found = true;
	      cout << "    " << _("Provided by enabled repository")   << ": " << (*it).name() << endl;

	    }
	  }
	  if ( ! found )
	  {
	    cout << ( ColorContext::MSG_WARNING << "    " << _("Not provided by any enabled repository") ) << endl;
	  }
	}
      }
      else
	cout << ": " << _("undefined") << endl;
    }

    printSummaryDesc( pool_item );

    // Print dependency lists if CLI requests it
    for ( auto && dep : cliSupportedDepTypes() )
    { if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( pool_item, dep ); }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
