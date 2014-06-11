/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <boost/format.hpp>

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
using boost::format;

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
} // namespace out
///////////////////////////////////////////////////////////////////

void printNVA(const ResObject::constPtr & res)
{
  cout << _("Name: ") << res->name() << endl;
  cout << _("Version: ") << res->edition().asString() << endl;
  cout << _("Arch: ") << res->arch().asString() << endl;
  cout << _("Vendor: ") << res->vendor() << endl;
}

void printSummaryDesc(const ResObject::constPtr & res)
{
  cout << _("Summary: ") << res->summary() << endl;
  cout << _("Description: ") << endl;
  Zypper::instance()->out().printRichText( res->description(), 2/*indented*/ );
}

/**
 *
 */
void printInfo(Zypper & zypper, const ResKind & kind)
{
  ResPool pool = God->pool();

  cout << endl;

  for(vector<string>::const_iterator nameit = zypper.arguments().begin();
      nameit != zypper.arguments().end(); ++nameit )
  {
    PoolQuery q;
    q.addKind(kind);
    q.addAttribute(sat::SolvAttr::name, *nameit);

    if ( !zypper.cOpts().count("match-substrings") )
    {
      q.setMatchExact();
    }

    if ( (*nameit).find_first_of("?*") != string::npos )
    {
      q.setMatchGlob();
    }

    if (q.empty())
    {
      // TranslatorExplanation E.g. "package 'zypper' not found."
      //! \todo use a separate string for each kind so that it is translatable.
      cout << "\n" << format(_("%s '%s' not found."))
          % kind_to_string_localized(kind, 1) % *nameit
          << endl;
    }
    else
    {
      for ( zypp::PoolQuery::Selectable_iterator it = q.selectableBegin();
	     it != q.selectableEnd(); it++)
      {
        // print info
        // TranslatorExplanation E.g. "Information for package zypper:"

        if (zypper.out().type() != Out::TYPE_XML)
        {
          string info = boost::str( format(_("Information for %s %s:"))
                                    % kind_to_string_localized(kind, 1)
                                    % (*it)->name() );

          cout << endl << info << endl;
          cout << string( mbs_width(info), '-' ) << endl;
        }

        if (kind == ResKind::package)
          printPkgInfo(zypper, *(*it));
        else if (kind == ResKind::patch)
          printPatchInfo(zypper, *(*it));
        else if (kind == ResKind::pattern)
          printPatternInfo(zypper, *(*it));
        else if (kind == ResKind::product)
          printProductInfo(zypper, *(*it));
        else
          // TranslatorExplanation %s = resolvable type (package, patch, pattern, etc - untranslated).
          zypper.out().info(
                            boost::str(format(_("Info for type '%s' not implemented.")) % kind));
      }
    }
  }
}


/**
 * Print package information.
 * <p>
 * Generates output like this:
<pre>
Catalog: system
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

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << theone.resolvable()->repository().asUserString() << endl;

  printNVA(theone.resolvable());

  // if running on SUSE Linux Enterprise, report unsupported packages
  Product::constPtr platform = God->target()->baseProduct();
  if (platform && platform->name().find("SUSE_SLE") != string::npos)
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
      cout << str::form(_("out-of-date (version %s installed)"),
			installed.resolvable()->edition().asString().c_str())
           << endl;
    }
    else
    {
      cout << _("up-to-date") << endl;
    }
  }
  else
    cout << _("not installed") << endl;

  cout << _("Installed Size: ") << theone.resolvable()->installSize() << endl;

  printSummaryDesc(theone.resolvable());

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
  printNVA(pool_item.resolvable());

  cout << _("Status: ") << string_patch_status(pool_item) << endl;

  Patch::constPtr patch = asKind<Patch>(pool_item.resolvable());
  cout << _("Category: ") << patch->category() << endl;
  cout << _("Severity: ") << patch->severity() << endl;
  cout << _("Created On: ") << patch->timestamp().asString() << endl;
  cout << _("Reboot Required: ") << (patch->rebootSuggested() ? _("Yes") : _("No")) << endl;

  if (!zypper.globalOpts().is_rug_compatible)
    cout << _("Package Manager Restart Required") << ": ";
  else
    cout << _("Restart Required: ");
  cout << (patch->restartSuggested() ? _("Yes") : _("No")) << endl;

  Patch::InteractiveFlags ignoreFlags = Patch::NoFlags;
  if (zypper.globalOpts().reboot_req_non_interactive)
    ignoreFlags |= Patch::Reboot;
  if ( zypper.cOpts().count("auto-agree-with-licenses") || zypper.cOpts().count("agree-to-third-party-licenses") )
    ignoreFlags |= Patch::License;

  cout << _("Interactive: ") << (patch->interactiveWhenIgnoring(ignoreFlags) ? _("Yes") : _("No")) << endl;

  printSummaryDesc(pool_item.resolvable());

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

Catalog: factory
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

  if ( !pool_item.resolvable()->isKind<Pattern>() )
    return;

  cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
       << pool_item.resolvable()->repository().asUserString() << endl;

  printNVA(pool_item.resolvable());

  cout << _("Installed: ") << (s.hasInstalledObj() ? _("Yes") : _("No")) << endl;
  cout << _("Visible to User: ") << (pool_item.resolvable()->asKind<Pattern>()->userVisible() ? _("Yes") : _("No")) << endl;

  printSummaryDesc(pool_item.resolvable());

  if (zypper.globalOpts().is_rug_compatible)
    return;

  // Print dependency lists if CLI requests it
  for ( auto && dep : cliSupportedDepTypes() )
  { if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( pool_item, dep ); }

  // show contents
  Table t;
  TableHeader th;
  th << _("S") << _("Name") << _("Type") << _("Dependency");
  t << th;

  //God->resolver()->solve();

  Pattern::constPtr pattern = asKind<Pattern>(pool_item.resolvable());
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
    cout << (zypper.globalOpts().is_rug_compatible ? _("Catalog: ") : _("Repository: "))
         << pool_item.resolvable()->repository().asUserString() << endl;

    printNVA(pool_item.resolvable());

    PoolItem installed;
    if (!s.installedEmpty())
      installed = s.installedObj();

    Product::constPtr product;
    if ( installed )
      product = asKind<Product>(installed);
    else
      product = asKind<Product>(pool_item.resolvable());

    cout << _("Flavor") << ": "  << product->flavor() << endl;

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
	colBad() <<  _("invalid CPE Name") << ": " << CpeId::NoThrowType::lastMalformed << endl;
    }
    {
      cout << _("Update Repositories");
      std::list<Repository::ContentIdentifier> l;
      unsigned cl = product->updateContentIdentifierSize( l );
      if ( cl )
      {
	cout << ": " << cl << endl;
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
	    colNote() << "    " << _("Not provided by any enabled repository")   << endl;
	  }
	}
      }
      else
	cout << ": " << _("undefined") << endl;
    }

    printSummaryDesc(pool_item.resolvable());

    // Print dependency lists if CLI requests it
    for ( auto && dep : cliSupportedDepTypes() )
    { if ( zypper.cOpts().count( asCliOption( dep ) ) ) printDepList( pool_item, dep ); }
  }
}

// Local Variables:
// c-basic-offset: 2
// End:
