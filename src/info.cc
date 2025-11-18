/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <optional>

#include <zypp/base/Algorithm.h>
#include <zypp/ZYpp.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>
#include <zypp/Pattern.h>
#include <zypp/Product.h>
#include <zypp/PoolQuery.h>
#include <zypp/base/LogTools.h>

#include "Zypper.h"
#include "main.h"
#include "Table.h"
#include "utils/misc.h"
#include "utils/text.h"
#include "utils/richtext.h"
#include "search.h"
#include "update.h"
#include "global-settings.h"

#include "info.h"

extern ZYpp::Ptr God;

void printPkgInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );
void printPatchInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );
void printPatternInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );
void printProductInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );
void printSrcPackageInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );
void printDefaultInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r );

///////////////////////////////////////////////////////////////////
namespace
{
  /**
   * Return the most interesting \ref PoolItems inside a \ref Selectable.
   *
   * Returns the installed item, the updateCandidate and \c theone which
   * is, unlike the first two, guaranteed to be not NULL.
   *
   * \c theone: Unless a specific version was selected by the user:
   * An updateCandidate is always better than any installed
   * object. If the best version is already installed try to look it up
   * in the repo it came from, otherwise use the installed one.
   * If there is no installed one either, use the candidate.
   *
   * \code
   * auto [installed, updateCand, theone] = theInterestingPoolItems( sel );
   * \endcode
   */
  inline auto theInterestingPoolItems( const ui::Selectable & sel, const std::optional<PoolItem> & theWanted_r )
  {
    PoolItem installed  { sel.installedObj() };
    PoolItem updateCand { sel.updateCandidateObj() };
    PoolItem theone     { theWanted_r ? *theWanted_r : updateCand };
    if ( not theone ) {
      theone = sel.identicalAvailableObj( installed );
      if ( not theone ) {
        theone = installed;
        if ( not theone )
          theone = sel.candidateObj();
      }
    }
    return std::make_tuple( installed, updateCand, theone );
  }

  inline std::string joinWords( std::string lhs, const std::string & rhs )
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

  inline void printSummaryDescDeps( const PoolItem & pi_r, PropertyTable & tab_r, const PrintInfoOptions &options_r )
  {
    // translators: property name; short; used like "Name: value"
    tab_r.add( _("Summary"),		pi_r.summary() );
    // translators: property name; short; used like "Name: value"
    tab_r.addDetail( _("Description"),	::printRichText( pi_r.description() ) );

    // dependencies according to flags and kind
    bool isPatch = pi_r.isKind<Patch>();

    for ( const auto & dep : std::vector<std::pair<InfoBits, Dep>>{
      { InfoBits::ShowProvides,    Dep::PROVIDES },
      { InfoBits::ShowRequires,    Dep::REQUIRES },
      { InfoBits::ShowConflicts,   Dep::CONFLICTS },
      { InfoBits::ShowObsoletes,   Dep::OBSOLETES },
      { InfoBits::ShowRecommends,  Dep::RECOMMENDS },
      { InfoBits::ShowSuggests,    Dep::SUGGESTS },
      { InfoBits::ShowSupplements, Dep::SUPPLEMENTS },
      { InfoBits::ShowEnhances,    Dep::ENHANCES },
    } )
    {
      if ( ( isPatch && ( dep.second == Dep::PROVIDES || dep.second == Dep::CONFLICTS ) )
        || ( options_r._flags.testFlag( dep.first ) ) )			// dep.asString - CLI option name
        tab_r.lst( dep.second.asUserString(), pi_r.dep( dep.second ) );	// dep.asUserString - i18n translation
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

  inline bool identIsInstalled( IdString ident_r )
  {
    if ( auto sel = ui::Selectable::get( ident_r ) )
      return not sel->installedEmpty();
    return false;
  }

  template <typename TContainer>
  inline bool anyIdentIsInstalled( const TContainer & idents_r )
  {
    for ( IdString ident : idents_r ) {
      if ( identIsInstalled( ident ) )
        return true;
    }
    return false;
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
  // PoolQuery does no COW :( - build and return the basic query
  inline PoolQuery printInfo_BasicQuery( Zypper & zypper, const PrintInfoOptions &options_r )
  {
    PoolQuery baseQ;
    if ( options_r._matchSubstrings )
    { baseQ.setMatchSubstring(); }
    else
    { baseQ.setMatchGlob(); }	// is Exact if no glob chars included in name
    if ( InitRepoSettings::instance()._repoFilter.size() )
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

  void checkVersioned( PoolQuery & q_r, const std::string & arg_r )
  {
    q_r.addAttribute( sat::SolvAttr::name, arg_r );
    if ( not q_r.empty() )
      return; // name

    static const zypp::str::regex rxVers { "^(.+)-([^-]+)$" };
    static str::smatch what;
    // check for version
    if ( zypp::str::regex_match( arg_r, what, rxVers ) ) {
      std::string name = what[1];
      std::string ver  = what[2];
      q_r.addAttribute( sat::SolvAttr::name, name );
      q_r.setEdition( Edition( ver ) );
      if ( not q_r.empty() )
        return; // name-version

      // check for version-release
      if ( zypp::str::regex_match( name, what, rxVers ) ) {
        name = what[1];
        q_r.addAttribute( sat::SolvAttr::name, name );
        q_r.setEdition( Edition( what[2], ver ) );
        if ( not q_r.empty() )
          return; // name-version-release
      }
    }
    // no match at all
  }

} // namespace
///////////////////////////////////////////////////////////////////

void printInfo( Zypper & zypper, const std::vector<std::string> &names_r, const PrintInfoOptions &options_r )
{
  zypper.out().gap();
  bool noMatches = true;

  for ( const std::string & rawarg : names_r )
  {
    // Use the right kind!
    KNSplit kn( rawarg );

    PoolQuery q( printInfo_BasicQuery( zypper, options_r ) );
    bool fallBackToAny = false;
    if ( kn._kind )
    {
      q.addKind( kn._kind );			// explicit kind in arg
    }
    else if ( !options_r._kinds.empty() )
    {
      for ( const auto & kind : options_r._kinds )	// wanted kinds via -t
        q.addKind( kind );
    }
    else
    {
      q.addKind( ResKind::package );
      fallBackToAny = true;			// Prefer packages, but fall back to any
    }

    // PED-11268: Check for an optional "-version" or "-version-release"if there
    // are no matches on name. If q is not empty afterwards, check q.edition().
    checkVersioned( q, kn._name );

    if ( q.empty() )
    {
      ResKind oneKind( kn._kind );
      if ( !oneKind )
      {
        if ( !options_r._kinds.empty() && options_r._kinds.size() == 1 )
          oneKind = *options_r._kinds.begin();
        else
          oneKind = ResKind::package;
      }
      // TranslatorExplanation E.g. "package 'zypper' not found."
      cout << "\n" << str::Format(_("%s '%s' not found.")) % kind_to_string_localized( oneKind, 1 ) % rawarg << endl;

      // hint to matches of different kind (preferPackages looked for any)
      PoolQuery h( printInfo_BasicQuery( zypper, options_r ) );
      h.addAttribute( sat::SolvAttr::name, kn._name );

      if ( h.empty() ) {
        continue;
      }
      else if ( !fallBackToAny ) {
        logOtherKindMatches( h, kn._name );
        continue;
      }
      else {
        q = h;
      }
    }

    for_( it, q.selectableBegin(), q.selectableEnd() )
    {
      if ( noMatches ) noMatches = false;
      const ui::Selectable & sel( *(*it) );
      std::optional<PoolItem> theWanted;
      if ( q.edition() ) {
        // An additional version constraint, find the wanted PoolItem
        const Edition & ed { q.edition() };
        for ( const auto & pi : sel.picklist() ) {
          if ( Edition::match( ed, pi.edition() ) == 0 ) {
            theWanted = pi;
            break;
          }
        }
      }

      if ( zypper.out().type() != Out::TYPE_XML )
      {
        std::string pkgtag = { sel.name() };
        if ( theWanted ) {
          pkgtag += "-";
          pkgtag += theWanted->edition().asString();
        }
        // TranslatorExplanation E.g. "Information for package zypper:"
        std::string info = str::Format(_("Information for %s %s:"))
                                 % kind_to_string_localized( sel.kind(), 1 )
                                 % pkgtag;

        cout << endl << info << endl;
        cout << std::string( mbs_width(info), '-' ) << endl;
      }

      if      ( sel.kind() == ResKind::package )	{ printPkgInfo( zypper, sel, theWanted, options_r ); }
      else if ( sel.kind() == ResKind::patch )		{ printPatchInfo( zypper, sel, theWanted, options_r ); }
      else if ( sel.kind() == ResKind::pattern )	{ printPatternInfo( zypper, sel, theWanted, options_r ); }
      else if ( sel.kind() == ResKind::product )	{ printProductInfo( zypper, sel, theWanted, options_r ); }
      else if ( sel.kind() == ResKind::srcpackage)	{ printSrcPackageInfo( zypper, sel, theWanted, options_r ); }
      else 						{ printDefaultInfo( zypper, sel, theWanted, options_r ); }
    }
  }

  if ( noMatches ) {
    zypper.out().info(_("No matching items found."), Out::QUIET );
    if ( !zypper.config().ignore_unknown ) {
      zypper.setExitInfoCode( ZYPPER_EXIT_INF_CAP_NOT_FOUND );
    }
  }
}

/** Information for kinds which don't have a extra needs... */
void printDefaultInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r  )
{
  auto [installed, updateCand, theone] = theInterestingPoolItems( s, theWanted_r );

  PropertyTable p;
  printCommonData( theone, p );

  printSummaryDescDeps( theone, p, options_r );
  zypper.out().info( str::Str() << p, Out::QUIET );
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
void printPkgInfo(Zypper & zypper, const ui::Selectable & s , const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r )
{
  auto [installed, updateCand, theone] = theInterestingPoolItems( s, theWanted_r );
  Package::constPtr package = theone->asKind<Package>();

  PropertyTable p;
  printCommonData( theone, p );

  //bnc#764147 Show Support Status always if not unknown, not only on SLE
  VendorSupportOption supportOpt = package->vendorSupport();
  if ( runningOnEnterprise() || supportOpt != VendorSupportOption::VendorSupportUnknown ) {
    if ( supportOpt == VendorSupportSuperseded ) {
      // jsc#OBS-301, jsc#PED-8014: add the superseding package(s)
      // and some hint how to proceed
      const std::pair<std::vector<IdString>,std::vector<std::string>> & supersededByItems { package->supersededByItems() };
      const std::vector<IdString> & superseding { supersededByItems.first };
      if ( not superseding.empty() ) {
        // add the known successor package(s)
        str::Str str;
        dumpRangeLine( str.stream() << asUserString( supportOpt ) << " ", superseding );
        p.add( _("Support Level"), HIGHLIGHTString( str ) );
        if ( anyIdentIsInstalled( superseding ) )
          p.addDetail( HIGHLIGHTString(_("The successor package is already installed.")) );
        else
          p.addDetail( HIGHLIGHTString(_("The successor package should be installed as soon as possible to receive continued support.")) );
      } else {
        const std::vector<std::string> & missing { supersededByItems.second };
        // kind of repo metadata oops (can't find the superseding package(s))
        str::Str str;
        dumpRangeLine( str.stream() << asUserString( supportOpt ) << " ", missing );
        p.add( _("Support Level"), HIGHLIGHTString( str ) );
        p.addDetail( HIGHLIGHTString(_("Unfortunately the successor package is not provided by any repository.")) );
        //
      }

    } else {
      p.add( _("Support Level"), asUserString( supportOpt ) );
    }

  }
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
  if ( const std::string & url { package->url() }; not url.empty() )
    p.add( _("Upstream URL"),	url );

  printSummaryDescDeps( theone, p, options_r );
  zypper.out().info( str::Str() << p, Out::QUIET );
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
void printPatchInfo(Zypper & zypper, const ui::Selectable & s , const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r )
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

  printSummaryDescDeps( theone, p, options_r );
  zypper.out().info( str::Str() << p, Out::QUIET );
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
namespace {
  struct PatternContentHelper
  {
    enum Rank { NONE=0, Req, Rec, Sug }; // Dep ranking

    struct Element
    {
      Rank _dep = NONE;
      Capability _cap;                              // the dependency we're looking for
      unsigned _num = 0U;                           // the number of providers
      std::optional<ui::Selectable::Ptr> _provider; // the provider to show (if any)

      explicit operator bool() const
      { return _dep; }

      // the table entires:
      std::string s() const
      { return _provider ? computeStatusIndicator( **_provider ) : std::string(); }

      std::string name() const
      { return _provider ? (*_provider)->ident().asString() : _cap.asString(); }

      std::string type() const
      { return _provider ? (*_provider)->kind().asString() : Dep::PROVIDES.asUserString(); }

      std::string depstr() const
      { return _dep == Req ? _("Required") : _dep == Rec ? _("Recommended") : _("Suggested"); }

      // Table ordering
      int order() const
      {
        int ret = _dep*2;
        if ( _provider && (*_provider)->kind() != ResKind::package )
          --ret;  // non-packages before packages
        return ret;
      }
    };

    std::map<std::string,Element> _content;

    // Elements will be added in order of relevance (Req, Rec, Sug).
    // => Add new ones only if a more relevant does not exist yet.
    //
    void add( const Capabilities & capset_r, Rank dep_r )
    {
      for ( const auto & c : capset_r ) {
        if ( str::startsWith( c.c_str(), "rpmlib(" ) )
          continue;

        sat::WhatProvides wp { c };       // wp: stay in scope
        auto providers = wp.selectable(); // iterator reference into wp!
        auto numproviders = providers.size();
        switch( numproviders ) {
          case 0:   // no provider (collect only if missing requires)
            if ( dep_r == Req )
              add( dep_r, c, numproviders );
            break;
          case 1:   // unique provider
            add( dep_r, c, numproviders, *providers.begin() );
            break;
          default:  // multiple providers
          {
            // Keeping it simple: Show the best installed provider
            // otherwise display the missing capability.
            ui::Selectable::Ptr ishow;
            for ( const auto & sel : providers ) {
              if ( sel->hasInstalledObj() ) {
                if ( c.id() == sel->ident().id() ) {
                  ishow = sel;
                  break;        // best: installed and same ident as the dependency
                }
                else if ( not ishow || sel->ident() < ishow->ident() ) {
                  ishow = sel;  // lex. least ident otherwise
                }
              }
            }
            if ( ishow )
              add( dep_r, c, numproviders, ishow );
            else
              add( dep_r, c, numproviders );
          }
          break;
        }
      }
    }

    // The depkeeper: we invent a self referring capability
    void add( const sat::Solvable & el_r, Rank dep_r )
    {
      add( dep_r, Capability(el_r.ident().id()), 1, ui::Selectable::get( el_r ) );
    }

  private:
    // An (usually installed) Selectable representing the Capability is added under it's ident
    void add( Rank dep_r, Capability cap_r, unsigned num_r, ui::Selectable::Ptr provider_r )
    {
      // translate a patterns- package back into it's pattern
      rewriteIfPatternsPackge( provider_r );

      Element & el = _content[provider_r->ident().asString()];
      if ( not el ) {
        el._provider = provider_r;
        el._dep = dep_r;
        el._cap = cap_r;
        el._num = num_r;
      }
    }
    // No or multiple providers (usually not installed)
    void add( Rank dep_r, Capability cap_r, unsigned num_r )
    {
      Element & el = _content[cap_r.asString()];
      if ( not el ) {
        el._dep = dep_r;
        el._cap = cap_r;
        el._num = num_r;
      }
    }
    void rewriteIfPatternsPackge( ui::Selectable::Ptr & provider_r )
    {
      // translate a patterns- package back into it's pattern
#if LIBZYPP_VERSION <= 173718
      static const Capability indicator { "pattern()" };
      if ( str::hasPrefix( provider_r->ident().c_str(), "pattern" ) && provider_r->theObj().dep_provides().matches(indicator) )
#else
      if ( str::hasPrefix( provider_r->ident().c_str(), "pattern" ) && provider_r->theObj().isPatternPackage() )
#endif
      {
        static const std::string cap { "autopattern() = " };
        sat::WhatProvides wp { Capability( cap+provider_r->ident().c_str() ) };
        if ( not wp.empty() )
          provider_r = *wp.selectableBegin();
      }
    }
  };

  inline std::ostream & operator<<( std::ostream & str, const PatternContentHelper::Element & obj )
  { return str << obj._dep << " " << ( obj._provider ? (*obj._provider)->hasInstalledObj() ? "i" : "u" : obj._num ? "*" : "-" ) << " " << obj.name(); }

  PatternContentHelper patternContentHelper( Pattern::constPtr pattern_r, bool addSuggests_r )
  {
    PatternContentHelper ret;
    sat::Solvable depKeeper( pattern_r->autoPackage() );  // (my required) patterns-package

    if ( depKeeper ) {
      // collect the depKeeper
      ret.add( depKeeper, PatternContentHelper::Req );
      // and expand it
      ret.add( depKeeper[Dep::REQUIRES],   PatternContentHelper::Req );
      ret.add( depKeeper[Dep::RECOMMENDS], PatternContentHelper::Rec );
      if ( addSuggests_r )
        ret.add( depKeeper[Dep::SUGGESTS], PatternContentHelper::Sug );
    }
    return ret;
  }
} // namespace

void printPatternInfo(Zypper & zypper, const ui::Selectable & s , const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r )
{
  auto [installed, updateCand, theone] = theInterestingPoolItems( s, theWanted_r );
  Pattern::constPtr pattern = theone->asKind<Pattern>();

  PropertyTable p;
  printCommonData( theone, p );

  // translators: property name; short; used like "Name: value"
  // translators: A pattern has a 1:1 relation to a .rpm package defining it's properties AKA the pattern's Buddy.
  p.add( _("Buddy Package"),		pattern->autoPackage() );
  // translators: property name; short; used like "Name: value"
  p.add( _("Installed"),		propertyInstalled( s.installedObj() ) );
  // translators: property name; short; used like "Name: value"
  p.add( _("Visible to User"),		pattern->userVisible() );

  printSummaryDescDeps( theone, p, options_r );

  {	// show contents
    bool showSuggests = options_r._flags.testFlag( InfoBits::ShowSuggests );

    PatternContentHelper contentHelper { patternContentHelper( pattern, showSuggests ) };
    const auto & content { contentHelper._content };

    if ( content.empty() ) {
      // translators: property name; short; used like "Name: value"
      p.addDetail( _("Contents"),	_("(empty)") );
    } else {
      Table t;
      t << ( TableHeader()
          /* translators: Table column header */	<< N_("S")
          /* translators: Table column header */	<< N_("Name")
          /* translators: Table column header */	<< N_("Type")
          /* translators: Table column header */	<< N_("Dependency") );

      for( const auto & [k,v] : content ) {
        TableRow tr;
        tr << v.s() << v.name() << v.type() << v.depstr();
        tr.userData( v.order() );
        t << std::move(tr);
      }
      t.sort(4);  // by userdata

      // translators: property name; short; used like "Name: value"
      p.addDetail( _("Contents"),	str::Str() << t );
    }
  }
  zypper.out().info( str::Str() << p, Out::QUIET );
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
void printProductInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r  )
{
  if ( zypper.out().type() == Out::TYPE_XML )
  {
    PoolItem theone( theWanted_r ? *theWanted_r : s.theObj() );
    Product::constPtr product = theone->asKind<Product>();
    cout << asXML( *product, theone.status().isInstalled() ) << endl;
    return;
  }

  auto [installed, updateCand, theone] = theInterestingPoolItems( s, theWanted_r );

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

  printSummaryDescDeps( theone, p , options_r );
  zypper.out().info( str::Str() << p, Out::QUIET );
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

  // Slightly different from computeStatusIndicator: 'i' if exactly the
  // source packages version is installed.
  const char * BinPkgStatus( const BinPkg & binPkg_r )
  {
    ui::Selectable::Ptr sel( ui::Selectable::get( binPkg_r.first ) );
    if ( sel->installedEmpty() )
      return "";
    for ( const PoolItem & pi : sel->installed() )
      if ( pi.edition() == binPkg_r.second )
        return "i";
    return "v";
    return computeStatusIndicator( *sel , binPkg_r.second );
  }

} // namespace
///////////////////////////////////////////////////////////////////

void printSrcPackageInfo(Zypper & zypper, const ui::Selectable & s, const std::optional<PoolItem> & theWanted_r, const PrintInfoOptions &options_r  )
{
  auto [installed, updateCand, theone] = theInterestingPoolItems( s, theWanted_r );
  SrcPackage::constPtr srcPackage = theone->asKind<SrcPackage>();

  PropertyTable p;
  printCommonData( theone, p );
  printSummaryDescDeps( theone, p, options_r );

  ///////////////////////////////////////////////////////////////////
  BinPkgs builtFrom( findBinPkgsBuiltFromSrc( theone ) );
  if ( ! builtFrom.empty() )
  {
    Table t;
    t << ( TableHeader()
        /* translators: Table column header */	<< N_("S")
        /* translators: Table column header */	<< N_("Name")
        /* translators: Table column header */	<< N_("Version") );

    for ( auto binPkg : builtFrom )
    { t << ( TableRow() << BinPkgStatus( binPkg ) << binPkg.first << binPkg.second ); }

    t.sort( 1 );

    // translators: property name; short; used like "Name: value"; is followed by a list of binary packages built from this source package
    p.addDetail( _("Builds binary package"),	str::Str() << t );
  }
  ///////////////////////////////////////////////////////////////////

  zypper.out().info( str::Str() << p, Out::QUIET );
}
