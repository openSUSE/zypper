/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <string.h>
#include <iostream>
#include <sstream>
#include <string_view>

#include <zypp/ZYppFactory.h>
#include <zypp/base/LogTools.h>
#include <zypp/base/Measure.h>
#include <zypp/base/DtorReset.h>
#include <zypp/ResPool.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/ui/Selectable.h>

#include "main.h"
#include "utils/text.h"
#include "utils/colors.h"
#include "utils/misc.h"
#include "Table.h"
#include "Zypper.h"

#include "Summary.h"
#include "utils/console.h"

// Suppress all application related summary messages.
// Summary messages produce lots of false reports about deleted applications. This
// is because the quality of the application related metadata is poor (better said
// gnome-centric, frequently changing and not fitting into the zypp model).
// While it's undecided whether we drop application support at all, we will at least
// avoid the disturbing messages.
#define WITH_APPLICATION_SUMMARY 0

// --------------------------------------------------------------------------

extern ZYpp::Ptr God;
namespace {
  using ResolverOption = bool (zypp::Resolver::*)() const;

  /** Hint if solver policy violations may be caused by --force-resolution */
  std::string _resolverForceResolutionHint( ResolverOption option_r )
  {
    std::string ret;
    if ( God->resolver()->forceResolve() && not ((*God->resolver()).*option_r)() ) {
      ret = MSG_WARNINGString( " [--force-resolution]" ).str();
    }
    return ret;
  }

  std::string resolverForceResolutionHintDowngrade()
  { return _resolverForceResolutionHint( &zypp::Resolver::allowDowngrade ); }

  std::string resolverForceResolutionHintArchChange()
  { return _resolverForceResolutionHint( &zypp::Resolver::allowArchChange ); }

  std::string resolverForceResolutionHintVendorChange()
  { return _resolverForceResolutionHint( &zypp::Resolver::allowVendorChange ); }


  // --------------------------------------------------------------------------
  struct SolvableSummaryItemFormater
  {
    SolvableSummaryItemFormater( ansi::Color color_r, bool withKind_r = false )
    : _configHighlight { Zypper::instance().config().color_pkglistHighlight }
    , _configAttr { Zypper::instance().config().color_pkglistHighlightAttribute }
    , _contextColor { color_r }
    , _withKind { withKind_r }
    {
      if ( _withKind )
        _extractFn = &SolvableSummaryItemFormater::extractIdent;
      if ( _configHighlight )
        _formatFn = &SolvableSummaryItemFormater::formatHighlightFirst;
      else if ( indeterminate(_configHighlight) )
        _formatFn = &SolvableSummaryItemFormater::formatHighlightIfChanged;
    }

    template<class TSolv>
    std::ostream & operator()( std::ostream & str, const sat::SolvableType<TSolv> & slv_r ) const
    { return operator()( str, slv_r.satSolvable() ); }

    std::ostream & operator()( std::ostream & str, sat::Solvable slv_r ) const
    { return quoteIf( str, (this->*_extractFn)( slv_r ) ); }

 private:
    std::ostream & quoteIf( std::ostream & str, const std::string & txt_r ) const
    {
      if ( txt_r.find_first_of( " " ) == std::string::npos )
        return (this->*_formatFn)( str, txt_r );
      else {
        static const HIGHLIGHTString quoteCh( "\"" );
        return (this->*_formatFn)( str << quoteCh, txt_r ) << quoteCh;
      }
    }

  private:
    using ExtractFn = std::string (SolvableSummaryItemFormater::*)( sat::Solvable slv_r ) const;
    using FormatFn = std::ostream & (SolvableSummaryItemFormater::*)( std::ostream &, const std::string & ) const;

  private:
    std::string extractName( sat::Solvable slv_r ) const
    { return slv_r.name(); }

    std::string extractIdent( sat::Solvable slv_r ) const
    { return slv_r.ident().asString(); }

  private:
    std::ostream & formatPlain( std::ostream & str, const std::string & txt_r ) const
    { return str << txt_r; }

    std::ostream & formatHighlightFirst( std::ostream & str, const std::string & txt_r ) const
    {
      std::string_view txt { txt_r };
      return str << ( _contextColor << _configAttr << txt.substr( 0, 1 ) ) << txt.substr( 1 ); }

    std::ostream & formatHighlightIfChanged( std::ostream & str, const std::string & txt_r ) const
    {
      if ( txt_r[0] != _first ) {
        _first = txt_r[0];
        return formatHighlightFirst( str, txt_r );
      }
      return formatPlain( str, txt_r );
    }

   private:
    TriBool _configHighlight;   ///< always/never or on a changed first char
    ansi::Color _configAttr;    ///< highlight attribute from config
    ansi::Color _contextColor;  ///< list color may be overwritten by _configAttr
    bool _withKind = false;     ///< prefix names by kind

    mutable char _first = '\0'; ///< for changed first char highlighting

    ExtractFn _extractFn = &SolvableSummaryItemFormater::extractName;
    FormatFn _formatFn = &SolvableSummaryItemFormater::formatPlain;
  };

  template <typename TContainer>  // sat::SolvableType
  std::ostream & writeSolvableList( std::ostream & str, const TContainer & container_r, ansi::Color color_r, bool withKind_r=false )
  {
    constexpr unsigned indent = 2;
    SolvableSummaryItemFormater format { color_r, withKind_r };

    std::ostringstream s;
    for ( const auto & slv : container_r ) {
      format( s, slv ) << " "; // don't mind trailing WS, it's eaten in mbs_write_wrapped
    }
    mbs_write_wrapped( str, s.str(), indent, get_screen_width() );
    return str << endl;
  }

  template <typename TContainer>  // std::pair<sat::SolvableType,std::string>
  std::ostream & writeSolvableDetailsList( std::ostream & str, const TContainer & container_r, ansi::Color color_r, bool withKind_r=false )
  {
    constexpr unsigned indent = 2;
    SolvableSummaryItemFormater format { color_r, withKind_r };


    std::ostringstream s;
    for ( const auto & pair : container_r ) {
      format( s, pair.first ) << " " << pair.second << endl;
    }
    mbs_write_wrapped( str, s.str(), indent, get_screen_width() );
    return str << endl;
  }
} // namespace

// --------------------------------------------------------------------------

bool Summary::ResPairNameCompare::operator()( const ResPair & p1, const ResPair & p2 ) const
{
  int ret = ::strcoll( p1.second->name().c_str(), p2.second->name().c_str() );
  if ( ret == 0 )
    return p1.second->edition() < p2.second->edition();
  return ret < 0;
}

// --------------------------------------------------------------------------

Summary::Summary( const ResPool & pool, SummaryHints summaryHints, const ViewOptions options )
: _summaryHints { std::move(summaryHints) }
, _viewop( options )
, _wrap_width( 80 )
, _force_no_color( false )
, _download_only( false )
, _dryRun( false )
{
  readPool( pool );
}

// --------------------------------------------------------------------------

struct ResNameCompare
{
  bool operator()( ResObject::constPtr r1, ResObject::constPtr r2 ) const
  {
    int ret = ::strcoll( r1->name().c_str(), r2->name().c_str() );
    if ( ret == 0 )
      return r1->edition() < r2->edition();
    return ret < 0;
  }
};

typedef std::map<Resolvable::Kind, std::set<ResObject::constPtr, ResNameCompare> > KindToResObjectSet;

// --------------------------------------------------------------------------

void Summary::readPool( const ResPool & pool )
{
  // reset stats
  _need_reboot_patch = false;
  _need_reboot_nonpatch = false;
  _need_restart = false;
  _inst_pkg_total = 0;

  _todownload = ByteCount();
  _incache = ByteCount();
  _inst_size_install = ByteCount();
  _inst_size_remove = ByteCount();

  _ctc.clear();

  // find multi-version packages, which actually have mult. versions installed
  for ( const ui::Selectable::Ptr & s : sat::Pool::instance().multiversion().selectable() )
  {
    if ( !s )
      continue;
    if ( s->installedSize() > 1 || ( s->installedSize() == 1 && s->toInstall() ) )
      _multiInstalled.insert( s->name() );
  }
  // collect resolvables to be installed/removed

  KindToResObjectSet to_be_installed;
  KindToResObjectSet to_be_removed;

  MIL << "Pool contains " << pool.size() << " items." << std::endl;
  DBG << "Install summary:" << endl;

  debug::Measure m;

  for_( it, pool.begin(), pool.end() )
  {
    if (it->status().isToBeInstalled() || it->status().isToBeUninstalled())
    {
      if ( it->isKind( ResKind::patch ) )
      {
        Patch::constPtr patch = asKind<Patch>(it->resolvable());

        // set the 'need reboot' flag
        if ( patch->rebootSuggested() )
        {
          _need_reboot_patch = true;
          _rebootNeeded[ResKind::patch].insert( ResPair( nullptr, patch ) );
        }
        else if ( patch->restartSuggested() )
          _need_restart = true;
      }

      if (it->status().isToBeInstalled())
      {
        DBG << "<install>   ";
        to_be_installed[it->kind()].insert(it->resolvable());

        if ( it->isKind( ResKind::package ) ) {
          Package::constPtr package = asKind<Package>( it->resolvable() );
          if ( package->isNeedreboot() ) {
            _need_reboot_nonpatch = true;
            _rebootNeeded[ResKind::package].insert( ResPair( nullptr, package ) );
          }
        }
      }
      if (it->status().isToBeUninstalled())
      {
        DBG << "<uninstall> ";
        to_be_removed[it->kind()].insert(it->resolvable());
      }
      DBG << *it << endl;
    }
  }

  if ( hasViewOption( PATCH_REBOOT_RULES ) ) {
    // bsc#1183268: Patch reboot-needed flag overrules included packages.
    // Work around the unfortunate case, where a maintenance patch's metadata
    // does not request a reboot, while some included package does.
    // The 'patch --skip-interactive' command operates based on the maintenance
    // metadata and skips patches that require a reboot. In this case we suppress
    // the reboot needed hint of included packages in order not to confuse the user.
    // The decision is debatable, but as long as maintenance provides a reboot-needed
    // flag for Patches, we will not overrule it by computing it from the included packages.
    if ( not _need_reboot_patch && _need_reboot_nonpatch ) {
      MIL << "PATCH_REBOOT_RULES overrules " << _rebootNeeded[ResKind::package].size() << " packages." << endl;
      _rebootNeeded[ResKind::package].clear();
      _need_reboot_nonpatch = false;
    }
  }

  for ( const auto & spkg : Zypper::instance().runtimeData().srcpkgs_to_install )
  { to_be_installed[ResKind::srcpackage].insert(spkg); }

  // total packages to download & install
  // (packages & srcpackages only - patches, patterns, and products are virtual)
  _inst_pkg_total =
    to_be_installed[ResKind::package].size() +
    to_be_installed[ResKind::srcpackage].size();

  m.elapsed();

  // iterate the to_be_installed to find installs/upgrades/downgrades + size info
  for_( it, to_be_installed.begin(), to_be_installed.end() )
  {
    for_( resit, it->second.begin(), it->second.end() )
    {
      ResObject::constPtr res(*resit);
      Package::constPtr pkg = asKind<Package>(res);

      if ( pkg )
      {
        switch ( pkg->vendorSupport() )
        {
          case VendorSupportUnknown:
            _supportUnknown[res->kind()].insert( ResPair( nullptr, res ) );
            break;
          case VendorSupportUnsupported:
            _supportUnsupported[res->kind()].insert( ResPair( nullptr, res ) );
            break;
          case VendorSupportACC:
            _supportNeedACC[res->kind()].insert( ResPair( nullptr, res ) );
            break;
          case VendorSupportSuperseded:
            _supportSuperseded[res->kind()].insert( ResPair( nullptr, res ) );
          default:
            // L1, L2 or L3 support are not reported
            break;
        }
      }

      // find in to_be_removed:
      bool upgrade_downgrade = false;
      for_( rmit, to_be_removed[res->kind()].begin(), to_be_removed[res->kind()].end() )
      {
        if ( res->name() == (*rmit)->name() )
        {
          ResPair rp( *rmit, res );

          // upgrade
          if ( res->edition() > (*rmit)->edition() )
          {
            // don't put multiversion packages to '_toupgrade', they will
            // always be reported as newly installed (and removed)
            if (_multiInstalled.find( res->name()) != _multiInstalled.end() )
              continue;

            _toupgrade[res->kind()].insert( rp );
            if ( res->arch() != (*rmit)->arch() )
              _tochangearch[res->kind()].insert( rp );
            if ( !VendorAttr::instance().equivalent(res->vendor(), (*rmit)->vendor()) )
              _tochangevendor[res->kind()].insert( rp );
          }
          // reinstall
          else if ( res->edition() == (*rmit)->edition() )
          {
            if ( res->arch() != (*rmit)->arch() )
              _tochangearch[res->kind()].insert( rp );
            else
              _toreinstall[res->kind()].insert( rp );
            if ( !VendorAttr::instance().equivalent( res->vendor(), (*rmit)->vendor() ) )
              _tochangevendor[res->kind()].insert( rp );
          }
          // downgrade
          else
          {
            // don't put multiversion packages to '_todowngrade', they will
            // always be reported as newly installed (and removed)
            if ( _multiInstalled.find( res->name() ) != _multiInstalled.end() )
              continue;

            _todowngrade[res->kind()].insert( rp );
            if ( res->arch() != (*rmit)->arch() )
              _tochangearch[res->kind()].insert( rp );
            if ( !VendorAttr::instance().equivalent(res->vendor(), (*rmit)->vendor()) )
              _tochangevendor[res->kind()].insert( rp );
          }

          _inst_size_install += res->installSize();
          _inst_size_remove += (*rmit)->installSize();

          // this turned out to be an upgrade/downgrade
          to_be_removed[res->kind()].erase( *rmit );
          upgrade_downgrade = true;
          break;
        }
      }

      if ( !upgrade_downgrade )
      {
        _toinstall[res->kind()].insert( ResPair( nullptr, res ) );
        _inst_size_install += res->installSize();
      }

      if ( pkg && pkg->isCached() )
        _incache += res->downloadSize();
      else
        _todownload += res->downloadSize();
    }
  }

  m.elapsed();

  // collect the rest (not upgraded/downgraded) of to_be_removed as '_toremove'
  // and decrease installed size change accordingly

  //bool _toremove_by_solver = false;
  for_( it, to_be_removed.begin(), to_be_removed.end() )
    for_( resit, it->second.begin(), it->second.end() )
    {
      /** \todo this does not work
      if (!_toremove_by_solver)
      {
        PoolItem pi(*resit);
        if (pi.status() == ResStatus::SOLVER)
          _toremove_by_solver = true;
      }*/
      _toremove[it->first].insert( ResPair( nullptr, *resit ) );
      _inst_size_remove += (*resit)->installSize();
    }

  m.elapsed();

  // *** notupdated ***

  // get all available updates, no matter if they are installable or break
  // some current policy
  KindToResPairSet candidates;
  ResKindSet kinds;
  kinds.insert( ResKind::package );
  kinds.insert( ResKind::product );
  for_( kit, kinds.begin(), kinds.end() )
  {
    for_( it, pool.proxy().byKindBegin(*kit), pool.proxy().byKindEnd(*kit) )
    {
      if ( !(*it)->hasInstalledObj() )
        continue;

      PoolItem candidate = (*it)->highestAvailableVersionObj();

      if ( !candidate )
        continue;
      if ( compareByNVRA( (*it)->installedObj(), candidate ) >= 0 )
        continue;
      // ignore higher versions with different arch (except noarch) bnc #646410
      if ( (*it)->installedObj().arch() != candidate.arch()
        && (*it)->installedObj().arch() != Arch_noarch
        && candidate.arch() != Arch_noarch )
        continue;
      // mutliversion packages do not end up in _toupgrade, so we need to remove
      // them from candidates if the candidate actually installs (bnc #629197)
      if ( _multiInstalled.find( candidate.name() ) != _multiInstalled.end()
        && candidate.status().isToBeInstalled() )
        continue;

      candidates[*kit].insert( ResPair( nullptr, candidate.resolvable() ) );
    }
    MIL << *kit << " update candidates: " << candidates[*kit].size() << endl;
    MIL << "to be actually updated: " << _toupgrade[*kit].size() << endl;
  }

  // compare available updates with the list of packages to be upgraded
  //
  // note: operator[] (kindToResPairSet[kind]) actually creates ResPairSet when
  //       used. This avoids bnc #594282 which occurred when there was
  //       for_(it, _toupgrade.begin(), _toupgrade.end()) loop used here and there
  //       were no upgrades for that kind.
  for_( kit, kinds.begin(), kinds.end() )
    std::set_difference( candidates[*kit].begin(), candidates[*kit].end(),
                         _toupgrade [*kit].begin(), _toupgrade [*kit].end(),
                         inserter( _notupdated[*kit], _notupdated[*kit].begin() ),
                         Summary::ResPairNameCompare() );

  // remove kinds with empty sets after the set_difference
  for ( KindToResPairSet::iterator it = _notupdated.begin(); it != _notupdated.end(); )
  {
    if (it->second.empty())
      _notupdated.erase(it++);
    else
      ++it;
  }
  for ( KindToResPairSet::iterator it = _toupgrade.begin(); it != _toupgrade.end(); )
  {
    if (it->second.empty())
      _toupgrade.erase(it++);
    else
      ++it;
  }

  m.stop();
}

// --------------------------------------------------------------------------

namespace
{
  inline unsigned numberOfPackagesIn( const Summary::KindToResPairSet & map_r )
  {
    // packages only - patches, patterns, and products
    // are virtual; srcpackages do not get removed
    auto it = map_r.find( ResKind::package );
    return( it == map_r.end() ? 0 : it->second.size() );
  }
} // namespace

unsigned Summary::packagesToInstall()	const	{ return numberOfPackagesIn( _toinstall ); }
unsigned Summary::packagesToUpgrade()	const	{ return numberOfPackagesIn( _toupgrade ); }
unsigned Summary::packagesToDowngrade()	const	{ return numberOfPackagesIn( _todowngrade ); }
unsigned Summary::packagesToReInstall()	const	{ return numberOfPackagesIn( _toreinstall ); }
unsigned Summary::packagesToRemove()	const	{ return numberOfPackagesIn( _toremove ); }

// --------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////
namespace
{
  inline std::string ResPair2Name( const Summary::ResPairSet::value_type & resp_r, bool withKind_r = false )
  {
    if ( resp_r.second->kind() == ResKind::product )
      // If two products are involved, show the old ones summary.
      // (The following product is going to be upgraded/downgraded:)
      return resp_r.first ? resp_r.first->summary() : resp_r.second->summary();

    return( withKind_r ? resp_r.second->ident().asString() : resp_r.second->name() );
  }

  /** bsc#1061384: add hint if product is better updated by a different command
   * Products buddy (the-release package) may provide some indicator telling that
   * a specific command should be used to update the product.
   * \code
   *   Provides product-update() == dup
   * \endcode
   */
  inline void addUpdateHint( TableRow & tr_r, std::list<std::string> & ctc_r, ResObject::constPtr obj_r )
  {
    if ( obj_r && obj_r->isKind<Product>() )
    {
      // Check if buddy release package contains some update indicator provides.
      if ( Zypper::instance().command() != ZypperCommand::DIST_UPGRADE_e )
      {
        static const Capability indicator( "product-update()", Rel::EQ, "dup" );
        if ( obj_r->asKind<Product>()->referencePackage().dep_provides().matches( indicator ) )
        {
          WAR << obj_r << " provides " << indicator << endl;
          // translator: '%1%' is a products name
          //             '%2%' is a command to call (like 'zypper dup')
          //             Both may contain whitespace and should be enclosed by quotes!
          std::string ctcmsg( str::Format( _("Product '%1%' requires to be upgraded by calling '%2%'!") ) % obj_r->summary() % DEFAULTString("zypper dup") );
          // ^^^ summary() for products like in ResPair2Name
          //     DEFAULTString to prevent the command string from being colored in MSG_WARNINGString below
          tr_r.addDetail( MSG_WARNINGString( ctcmsg ) );
          ctc_r.push_back( std::move(ctcmsg) );
        }
      }
    }
  }
} // namespace
///////////////////////////////////////////////////////////////////
bool Summary::writeResolvableList( std::ostream & out,
                                   const ResPairSet & resolvables,
                                   ansi::Color color,
                                   unsigned maxEntires_r,
                                   bool withKind_r )
{
  bool ret = true;	// whether the complete list was written, or maxEntires_r clipped

  if ( (_viewop & DETAILS) == 0 )
  {
    static const HIGHLIGHTString quoteCh( "\"" );

    TriBool pkglistHighlight = Zypper::instance().config().color_pkglistHighlight;
    ansi::Color pkglistHighlightAttribute = Zypper::instance().config().color_pkglistHighlightAttribute;
    char firstCh = 0;

    std::ostringstream s;
    unsigned relevant_entries = 0;
    for ( const ResPair & respair : resolvables )
    {
      // name
      const std::string & name( ResPair2Name( respair, withKind_r ) );
      ++relevant_entries;
      if ( maxEntires_r && relevant_entries > maxEntires_r )
        continue;

      // quote names with spaces
      bool quote = name.find_first_of( " " ) != std::string::npos;

      // quote?
      if ( quote ) s << quoteCh;

      // highlight 1st char?
      if ( pkglistHighlight || ( indeterminate(pkglistHighlight) && name[0] != firstCh ) )
      {
        s << ( color << pkglistHighlightAttribute << name[0] ) << name.c_str()+1;
        if ( indeterminate(pkglistHighlight) )
           firstCh = name[0];
      }
      else
      {
        s << name;
      }

      // quote?
      if ( quote ) s << quoteCh;

      // version (if multiple versions are present)
      if ( _multiInstalled.find( respair.second->name() ) != _multiInstalled.end() )
      {
        if ( respair.first && respair.first->edition() != respair.second->edition() )
          s << "-" << respair.first->edition().asString()
            << "->" << respair.second->edition().asString();
        else
          s << "-" << respair.second->edition().asString();
      }

      s << " ";
    }
    if ( maxEntires_r && relevant_entries > maxEntires_r )
    {
      relevant_entries -= maxEntires_r;
      // translators: Appended when clipping a long enumeration:
      // "ConsoleKit-devel ConsoleKit-doc ... and 20828 more items."
      s << ( color << str::Format(PL_( "... and %1% more item.",
                                       "... and %1% more items.",
                                       relevant_entries) ) % relevant_entries );
      ret = false;
    }
    mbs_write_wrapped( out, s.str(), 2, _wrap_width );
    out << endl;
    return ret;
  }

  Table t;
  t.lineStyle(TableLineStyle::none);
  t.margin(2);
  t.wrap(0);

  unsigned relevant_entries = 0;
  for ( const ResPair & respair : resolvables )
  {
    ++relevant_entries;
    std::string name = ResPair2Name( respair, withKind_r );
    if ( maxEntires_r && relevant_entries > maxEntires_r )
      continue;

    // version (if multiple versions are present)
    if ( !(_viewop & SHOW_VERSION) && _multiInstalled.find( respair.second->name() ) != _multiInstalled.end() )
    {
      if ( respair.first && respair.first->edition() != respair.second->edition() )
        name += std::string("-") + respair.first->edition().asString()
             + "->" + respair.second->edition().asString();
      else
        name += std::string("-") + respair.second->edition().asString();
    }

    TableRow tr;
    tr << name;
    if ( _viewop & SHOW_VERSION )
    {
      if ( respair.first && respair.first->edition() != respair.second->edition() )
      {
        tr << respair.first->edition().asString() + " -> " +
              respair.second->edition().asString();
        // bsc#1061384: add hint if product is better updated by a different command
        addUpdateHint( tr, _ctc, respair.second );
      }
      else
        tr << respair.second->edition().asString();
    }
    if ( _viewop & SHOW_ARCH )
    {
      if ( respair.first && respair.first->arch() != respair.second->arch() )
        tr << respair.first->arch().asString() + " -> " +
              respair.second->arch().asString();
      else
        tr << respair.second->arch().asString();
    }
    if ( _viewop & SHOW_REPO )
    {
      // we do not know about repository changes, only show the repo from
      // which the package will be installed
      tr << respair.second->repoInfo().asUserString();
    }
    if ( _viewop & SHOW_VENDOR )
    {
      if ( respair.first && ! VendorAttr::instance().equivalent( respair.first->vendor(), respair.second->vendor() ) )
        tr << respair.first->vendor() + " -> " + respair.second->vendor();
      else
        tr << respair.second->vendor();
    }
    t << std::move(tr);
  }
  out << t;
  if ( maxEntires_r && relevant_entries > maxEntires_r )
  {
    relevant_entries -= maxEntires_r;
    // translators: Appended when clipping a long enumeration:
    // "ConsoleKit-devel ConsoleKit-doc ... and 20828 more items."
    out << ( color << str::Format(PL_( "... and %1% more item.",
                                       "... and %1% more items.",
                                       relevant_entries) ) % relevant_entries ) << endl;
    ret = false;
  }

  return ret;
}

// --------------------------------------------------------------------------

void Summary::writeNewlyInstalled( std::ostream & out )
{
  for_( it, _toinstall.begin(), _toinstall.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following NEW package is going to be installed:",
        "The following %d NEW packages are going to be installed:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following NEW patch is going to be installed:",
        "The following %d NEW patches are going to be installed:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following NEW pattern is going to be installed:",
        "The following %d NEW patterns are going to be installed:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following NEW product is going to be installed:",
        "The following %d NEW products are going to be installed:",
        it->second.size());
    else if ( it->first == ResKind::srcpackage )
      label = PL_(
        "The following source package is going to be installed:",
        "The following %d source packages are going to be installed:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to be installed:",
        "The following %d applications are going to be installed:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::POSITIVE << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::POSITIVE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeRemoved( std::ostream & out )
{
  DtorReset guard( _viewop );
  unsetViewOption( SHOW_REPO ); // never show repo here, it's always @System

  for_( it, _toremove.begin(), _toremove.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to be REMOVED:",
        "The following %d packages are going to be REMOVED:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to be REMOVED:",
        "The following %d patches are going to be REMOVED:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to be REMOVED:",
        "The following %d patterns are going to be REMOVED:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is going to be REMOVED:",
        "The following %d products are going to be REMOVED:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to be REMOVED:",
        "The following %d applications are going to be REMOVED:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::NEGATIVE << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::NEGATIVE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeUpgraded( std::ostream & out )
{
  for_( it, _toupgrade.begin(), _toupgrade.end() )
  {
    DtorReset guard( _viewop );		// need special viewopts for products
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to be upgraded:",
        "The following %d packages are going to be upgraded:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to be upgraded:",
        "The following %d patches are going to be upgraded:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to be upgraded:",
        "The following %d patterns are going to be upgraded:",
        it->second.size() );
    else if ( it->first == ResKind::product )
    {
      label = PL_(
        "The following product is going to be upgraded:",
        "The following %d products are going to be upgraded:",
        it->second.size() );
      setViewOption( SHOW_VERSION );	// always show version for products
    }
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to be upgraded:",
        "The following %d applications are going to be upgraded:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::POSITIVE << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::POSITIVE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeDowngraded( std::ostream & out )
{
  for_( it, _todowngrade.begin(), _todowngrade.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to be downgraded:",
        "The following %d packages are going to be downgraded:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to be downgraded:",
        "The following %d patches are going to be downgraded:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to be downgraded:",
        "The following %d patterns are going to be downgraded:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is going to be downgraded:",
        "The following %d products are going to be downgraded:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to be downgraded:",
        "The following %d applications are going to be downgraded:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::NEGATIVE << label ) << resolverForceResolutionHintDowngrade() << endl;
    writeResolvableList( out, it->second, ColorContext::NEGATIVE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeReinstalled( std::ostream & out )
{
  for_( it, _toreinstall.begin(), _toreinstall.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to be reinstalled:",
        "The following %d packages are going to be reinstalled:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to be reinstalled:",
        "The following %d patches are going to be reinstalled:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to be reinstalled:",
        "The following %d patterns are going to be reinstalled:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is going to be reinstalled:",
        "The following %d products are going to be reinstalled:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
    {
      // Suppress 'reinstalled' message for applications.
      // As applications don't have an edition, they are not updated
      // if the package updates. Technically every app-package update
      // is an 'application reinstall', because the providing package
      // changes.
      continue;
      label = PL_(
        "The following application is going to be reinstalled:",
        "The following %d applications are going to be reinstalled:",
        it->second.size() );
    }
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::CHANGE << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::CHANGE );
  }
}

// --------------------------------------------------------------------------

void Summary::collectInstalledRecommends( const ResObject::constPtr & obj )
{
  Capabilities rec = obj->dep_recommends();
  for_( capit, rec.begin(), rec.end() )
  {
    sat::WhatProvides q( *capit );
    // not using selectables here: matching found resolvables against those
    // in the _toinstall set (the ones selected by the solver)
    for_( sit, q.begin(), q.end() )
    {
      if ( sit->isSystem() ) // is it necessary to have the system solvable?
        continue;
      if ( sit->name() == obj->name() )
        continue; // ignore self-recommends (should not happen, though)

      XXX << "rec: " << *sit << endl;
      ResObject::constPtr recobj = makeResObject( *sit );
      ResPairSet::const_iterator match = _toinstall[sit->kind()].find( ResPair( nullptr, recobj ) );
      if ( match != _toinstall[sit->kind()].end() )
      {
        if ( _recommended[sit->kind()].insert( *match ).second )
          collectInstalledRecommends( recobj );
        break;
      }
    }
  }

  Capabilities req = obj->dep_requires();
  for_( capit, req.begin(), req.end() )
  {
    sat::WhatProvides q( *capit );
    for_( sit, q.begin(), q.end() )
    {
      if ( sit->isSystem() ) // is it necessary to have the system solvable?
        continue;
      if ( sit->name() == obj->name() )
        continue; // ignore self-requires
      XXX << "req: " << *sit << endl;
      ResObject::constPtr reqobj = makeResObject( *sit );
      ResPairSet::const_iterator match = _toinstall[sit->kind()].find( ResPair( nullptr, reqobj ) );
      if ( match != _toinstall[sit->kind()].end() )
      {
        if ( _required[sit->kind()].insert( *match ).second )
          collectInstalledRecommends( reqobj );
        break;
      }
    }
  }
}

// --------------------------------------------------------------------------

static void collectNotInstalledDeps( const Dep & dep, const ResObject::constPtr & obj, Summary::KindToResPairSet & result )
{
  static std::vector<ui::Selectable::Ptr> tmp;	// reuse capacity
  //DBG << obj << endl;
  Capabilities req = obj->dep( dep );
  for_( capit, req.begin(), req.end() )
  {
    tmp.clear();
    sat::WhatProvides q( *capit );
    for_( it, q.selectableBegin(), q.selectableEnd() )
    {
      if ( (*it)->name() == obj->name() )
        continue;		// ignore self-deps

      if ( (*it)->offSystem() )
      {
        if ( (*it)->toDelete() )
          continue;		// ignore explicitly deleted
        tmp.push_back( (*it) );	// remember uninstalled
      }
      else
      {
        // at least one of the recommendations is/gets installed: discard all
        tmp.clear();
        break;
      }
    }
    if ( !tmp.empty() )
    {
      // collect remembered ones
      for_( it, tmp.begin(), tmp.end() )
      {
        //DBG << dep << " :" << (*it)->onSystem() << ": " << dump(*(*it)) << endl;
        result[(*it)->kind()].insert( Summary::ResPair( nullptr, (*it)->candidateObj() ) );
      }
    }
  }
}

// --------------------------------------------------------------------------

void Summary::writeRecommended( std::ostream & out )
{
  // lazy-compute the installed recommended objects
  if (_recommended.empty() )
  {
    ResObject::constPtr obj;
    for_( kindit, _toinstall.begin(), _toinstall.end() )
      for_( it, kindit->second.begin(), kindit->second.end() )
        // collect recommends of all packages request by user
        if ( it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER )
          collectInstalledRecommends( it->second );
  }

  // lazy-compute the not-to-be-installed recommended objects
  if ( _noinstrec.empty() )
  {
    ResObject::constPtr obj;
    for_( kindit, _toinstall.begin(), _toinstall.end() )
      for_( it, kindit->second.begin(), kindit->second.end() )
        if ( it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER )
          collectNotInstalledDeps( Dep::RECOMMENDS, it->second, _noinstrec );
  }

  for_( it, _recommended.begin(), _recommended.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following recommended package was automatically selected:",
        "The following %d recommended packages were automatically selected:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following recommended patch was automatically selected:",
        "The following %d recommended patches were automatically selected:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following recommended pattern was automatically selected:",
        "The following %d recommended patterns were automatically selected:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following recommended product was automatically selected:",
        "The following %d recommended products were automatically selected:",
        it->second.size() );
    else if ( it->first == ResKind::srcpackage )
      label = PL_(
        "The following recommended source package was automatically selected:",
        "The following %d recommended source packages were automatically selected:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following recommended application was automatically selected:",
        "The following %d recommended applications were automatically selected:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::POSITIVE << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::POSITIVE );
  }

  for_( it, _noinstrec.begin(), _noinstrec.end() )
  {
    std::string label( "%d" );;
    // For packages, check the reason for not being installed. One reason can be that
    // the solver is told to install only required packages. If not, a package might be
    // unwanted because the user has removed it manually (added to /var/lib/zypp/SoftLocks)
    // or it will not be installed due to conflicts/dependency issues.
    if (it->first == ResKind::package)
    {
      Resolver_Ptr resolver = getZYpp()->resolver();

      ResPairSet softLocked;
      ResPairSet conflicts;
      ResPairSet notRequired;
      for_( pair_it, it->second.begin(), it->second.end() )
      {
        if ( resolver->onlyRequires() ) // only required packages will be installed
        {
          notRequired.insert( *pair_it );
        }
        else    // recommended packages should be installed - but...
        {
          if ( pair_it->second->poolItem().status().isSoftLocked() )
          {
            softLocked.insert( *pair_it );
          }
          else
          {
            conflicts.insert( *pair_it );
          }
        }
      }

      if ( resolver->onlyRequires() )
      {
        label = PL_( "The following package is recommended, but will not be installed (only required packages will be installed):",
                     "The following %d packages are recommended, but will not be installed (only required packages will be installed):",
                     it->second.size() );
        label = str::form( label.c_str(), it->second.size() );

        out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
        writeResolvableList( out, notRequired, ColorContext::HIGHLIGHT );
      }
      else
      {
        if ( !softLocked.empty() )
        {
          label = PL_( "The following package is recommended, but will not be installed because it's unwanted (was manually removed before):",
                       "The following %d packages are recommended, but will not be installed because they are unwanted (were manually removed before):",
                       it->second.size() );
          label = str::form( label.c_str(), it->second.size() );

          out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
          writeResolvableList( out, softLocked, ColorContext::HIGHLIGHT );
        }
        if ( !conflicts.empty() )
        {
          label = PL_( "The following package is recommended, but will not be installed due to conflicts or dependency issues:",
                       "The following %d packages are recommended, but will not be installed due to conflicts or dependency issues:",
                       it->second.size() );
          label = str::form( label.c_str(), it->second.size() );

          out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
          writeResolvableList( out, conflicts, ColorContext::HIGHLIGHT );
        }
      }
    }
    else
    {
      if ( it->first == ResKind::patch )
        label = PL_( "The following patch is recommended, but will not be installed:",
                     "The following %d patches are recommended, but will not be installed:",
                     it->second.size() );
      else if ( it->first == ResKind::pattern )
        label = PL_( "The following pattern is recommended, but will not be installed:",
                     "The following %d patterns are recommended, but will not be installed:",
                     it->second.size() );
      else if ( it->first == ResKind::product )
        label = PL_( "The following product is recommended, but will not be installed:",
                     "The following %d products are recommended, but will not be installed:",
                     it->second.size() );
      else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
        label = PL_( "The following application is recommended, but will not be installed:",
                     "The following %d applications are recommended, but will not be installed:",
                     it->second.size() );
#else
        continue;
#endif
      label = str::form( label.c_str(), it->second.size() );

      out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
      writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
    }
  }
}

// --------------------------------------------------------------------------

void Summary::writeSuggested( std::ostream & out )
{
  if ( _noinstsug.empty() )
  {
    ResObject::constPtr obj;
    for_( kindit, _toinstall.begin(), _toinstall.end() )
      for_( it, kindit->second.begin(), kindit->second.end() )
        // collect recommends of all packages request by user
        if ( it->second->poolItem().status().getTransactByValue() != ResStatus::SOLVER )
          collectNotInstalledDeps( Dep::SUGGESTS, it->second, _noinstsug );
  }

  for_( it, _noinstsug.begin(), _noinstsug.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is suggested, but will not be installed:",
        "The following %d packages are suggested, but will not be installed:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is suggested, but will not be installed:",
        "The following %d patches are suggested, but will not be installed:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is suggested, but will not be installed:",
        "The following %d patterns are suggested, but will not be installed:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is suggested, but will not be installed:",
        "The following %d products are suggested, but will not be installed:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is suggested, but will not be installed:",
        "The following %d applications are suggested, but will not be installed:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
  }
}

// --------------------------------------------------------------------------

void Summary::writeChangedArch( std::ostream & out )
{
  DtorReset guard( _viewop );
  setViewOption( SHOW_ARCH ); // always show arch here

  for_( it, _tochangearch.begin(), _tochangearch.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to change architecture:",
        "The following %d packages are going to change architecture:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to change architecture:",
        "The following %d patches are going to change architecture:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to change architecture:",
        "The following %d patterns are going to change architecture:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is going to change architecture:",
        "The following %d products are going to change architecture:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to change architecture:",
        "The following %d applications are going to change architecture:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::CHANGE << label ) << resolverForceResolutionHintArchChange() << endl;
    writeResolvableList( out, it->second, ColorContext::CHANGE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeChangedVendor( std::ostream & out )
{
  DtorReset guard( _viewop );
  setViewOption( SHOW_VENDOR ); // always show vendor here

  for_( it, _tochangevendor.begin(), _tochangevendor.end() )
  {
    std::string label( "%d" );
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is going to change vendor:",
        "The following %d packages are going to change vendor:",
        it->second.size() );
    else if ( it->first == ResKind::patch )
      label = PL_(
        "The following patch is going to change vendor:",
        "The following %d patches are going to change vendor:",
        it->second.size() );
    else if ( it->first == ResKind::pattern )
      label = PL_(
        "The following pattern is going to change vendor:",
        "The following %d patterns are going to change vendor:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product is going to change vendor:",
        "The following %d products are going to change vendor:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application is going to change vendor:",
        "The following %d applications are going to change vendor:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::CHANGE << label ) << resolverForceResolutionHintVendorChange() << endl;
    writeResolvableList( out, it->second, ColorContext::CHANGE );
  }
}

// --------------------------------------------------------------------------

void Summary::writeSupportUnknown( std::ostream & out )
{
  for_( it, _supportUnknown.begin(), _supportUnknown.end() )
  {
    std::string label( "%d" );
    // we only look at vendor support in packages
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package has no support information from its vendor:",
        "The following %d packages have no support information from their vendor:",
        it->second.size() );
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
  }
}

void Summary::writeSupportUnsupported( std::ostream & out )
{
  for_( it, _supportUnsupported.begin(), _supportUnsupported.end() )
  {
    std::string label( "%d" );
    // we only look at vendor support in packages
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package is not supported by its vendor:",
        "The following %d packages are not supported by their vendor:",
        it->second.size() );
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
  }
}

void Summary::writeSupportNeedACC( std::ostream & out )
{
  for_( it, _supportNeedACC.begin(), _supportNeedACC.end() )
  {
    std::string label( "%d" );
    // we only look at vendor support in packages
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package needs additional customer contract to get support:",
        "The following %d packages need additional customer contract to get support:",
        it->second.size() );
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
  }
}

void Summary::writeSupportSuperseded( std::ostream & out )
{
  auto highlightColor = ColorContext::HIGHLIGHT;
  auto supersedingAsString = []( Package::constPtr package_r ) -> std::string {
    const std::pair<std::vector<IdString>,std::vector<std::string>> & supersededByItems { package_r->supersededByItems() };
    const std::vector<IdString> & superseding { supersededByItems.first };
    str::Str ret;
    ret << _("was superseded by");
    if ( not superseding.empty() )  // names resolved to existing packages
      dumpRange( ret.stream(), superseding.begin(), superseding.end(), " ", "", ", ", "", "" );
    else  // print at least the plain names
      dumpRange( ret.stream(), supersededByItems.second.begin(), supersededByItems.second.end(), " ", "", ", ", "", "" );
    return ret.str();
  };

  for_( it, _supportSuperseded.begin(), _supportSuperseded.end() )
  {
    // we only look at vendor support in packages
    if ( it->first == ResKind::package ) {
      std::string label = PL_(
        "The following package was discontinued and has been superseded by a new package with a different name.",
        "The following %d packages were discontinued and have been superseded by a new packages with different names.",
        it->second.size() );
      label = str::form( label.c_str(), it->second.size() );
      std::string labelHint = PL_(
        "The successor package should be installed as soon as possible to receive continued support:",
        "The successor packages should be installed as soon as possible to receive continued support:",
        it->second.size() );
      out << endl << ( highlightColor << label << endl << labelHint ) << endl;

      std::vector<std::pair<sat::Solvable,std::string>> itemsdetails;
      for ( const auto & pair : it->second ) {
        Package::constPtr package = pair.second->asKind<Package>();
        itemsdetails.push_back( std::make_pair( package->satSolvable(), supersedingAsString( package ) ) );
      }
      writeSolvableDetailsList( out, itemsdetails, highlightColor );
    }
  }
}

// --------------------------------------------------------------------------

void Summary::writeNotUpdated( std::ostream & out )
{
  for_( it, _notupdated.begin(), _notupdated.end() )
  {
    std::string label( "%d" );
    // we only look at update candidates for packages and products
    if ( it->first == ResKind::package )
      label = PL_(
        "The following package update will NOT be installed:",
        "The following %d package updates will NOT be installed:",
        it->second.size() );
    else if ( it->first == ResKind::product )
      label = PL_(
        "The following product update will NOT be installed:",
        "The following %d product updates will NOT be installed:",
        it->second.size() );
    else if ( it->first == ResKind::application )
#if ( WITH_APPLICATION_SUMMARY )
      label = PL_(
        "The following application update will NOT be installed:",
        "The following %d application updates will NOT be installed:",
        it->second.size() );
#else
      continue;
#endif
    label = str::form( label.c_str(), it->second.size() );

    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;
    writeResolvableList( out, it->second, ColorContext::HIGHLIGHT );
  }
}

void Summary::writeLocked( std::ostream & out )
{
  ResPairSet instlocks;	// locked + installed
  ResPairSet avidents;	// avaialble locked
  ResPoolProxy selPool( ResPool::instance().proxy() );

  for_( it, selPool.begin(), selPool.end() )
  {
    if ( (*it)->locked() )	// NOTE: this does not cover partial locks (not all instances locked)
    {
      if ( (*it)->hasInstalledObj() )
       for_( iit, (*it)->installedBegin(), (*it)->installedEnd() )
         instlocks.insert( ResPair( nullptr, *iit ) );
      else
       avidents.insert( ResPair( nullptr, (*it)->theObj() ) );
    }
  }
  if ( ! ( instlocks.empty() && avidents.empty() ) )
  {
    std::string label = PL_(
      "The following item is locked and will not be changed by any action:",
      "The following %d items are locked and will not be changed by any action:",
      ( instlocks.size() + avidents.size() )
    );
    label = str::form( label.c_str(), instlocks.size() + avidents.size() );
    out << endl << ( ColorContext::HIGHLIGHT << label ) << endl;

    bool wroteAll = true;
    if ( ! avidents.empty() )
    {
      DtorReset guard( _viewop );
      _viewop = DEFAULT;	// always as plain name list
      // translators: used as 'tag:' (i.e. followed by ':')
      out << " " << _("Available") << ':' << endl;
      wroteAll &= writeResolvableList( out, avidents, ColorContext::HIGHLIGHT, 100, /*withKind*/true );
    }
    if ( ! instlocks.empty() )
    {
      // translators: used as 'tag:' (i.e. followed by ':')
      out << " " << _("Installed") << ':' << endl;
      wroteAll &= writeResolvableList( out, instlocks, ColorContext::HIGHLIGHT, 100, /*withKind*/true );
    }
    if ( !wroteAll )
    {
      out << " " << str::Format(_("Run '%1%' to see the complete list of locked items.")) % "zypper locks -s" << endl;
    }
  }
}

void Summary::writeRebootNeeded( std::ostream & out )
{
  auto generateLabel = [] ( zypp::ResKind kind, size_t count ) {
    if ( kind == ResKind::patch )
      return str::form(PL_("The following patch requires a system reboot:",
                           "The following %d patches require a system reboot:",
                          count ), static_cast<int>( count ) );
    else if ( kind == ResKind::package )
      return str::form(PL_("The following package requires a system reboot:",
                           "The following %d packages require a system reboot:",
                          count ), static_cast<int>( count ) );
    return std::string();
  };

  auto checkAndPrintRebootNeeded = [ this, &generateLabel, &out ] ( zypp::ResKind kind ) {
    const ResPairSet & resolvables = _rebootNeeded[kind];
    if ( ! resolvables.empty() ) {
      size_t count = resolvables.size();
      out << endl << ( ColorContext::MSG_WARNING << generateLabel( kind, count ) ) << endl;
      writeResolvableList( out, resolvables, ColorContext::MSG_WARNING );
    }
  };

  checkAndPrintRebootNeeded( ResKind::patch );
  checkAndPrintRebootNeeded( ResKind::package );
}

void Summary::writeAutoResolved( std::ostream & out )
{
  if ( _summaryHints.haveSkippedPatches() ) {
    std::string label { _("Skipped needed patches which do not apply without conflict:") };
    out << endl << ( ColorContext::MSG_WARNING << label << " [-skip-not-applicable-patches]" ) << endl;
    writeSolvableList( out, _summaryHints.skippedPatchesSet(), ColorContext::MSG_WARNING );
  }
}

// --------------------------------------------------------------------------

void Summary::writeDownloadAndInstalledSizeSummary( std::ostream & out )
{
  if ( !_inst_pkg_total && _toremove.empty() )
    return; // nothing to do, keep silent

  auto fmtBC = []( ByteCount bc_r, const char * sign = " " ) {
    return sign + bc_r.asString( 7 );
  };

  out << endl;

  if ( _todownload || _incache ) {
    //   Package download size:   99.2 MiB
    // or
    //   Package download size:
    //                 |     105.7 MiB  overall download size
    //       99.2 MiB  |  -    6.4 MiB  already in cache
    const auto & tagstr = _("Package download size:");
    if ( not _incache ) {
      out << tagstr << fmtBC( _todownload+_incache ) << endl;
    } else {
      out << tagstr << endl;
      Table t;
      t.lineStyle( TableLineStyle::none );
      // translator: rendered as "105.7 MiB  overall download size"
      t << ( TableRow() << ""                   <<"|"<< fmtBC( _todownload+_incache ) << _("overall package size") );
      // translator: rendered as "  6.4 MiB  already in cache"
      t << ( TableRow() << fmtBC( _todownload ) <<"|"<< fmtBC( _incache, "-" ) << _("already in cache") );
      out << t; // NL terminated
    }
    out << endl;
  }

  if ( _download_only ) {
    out << _("Download only.") << endl;
  } else {
    // Package install size change:
    //               |     349.1 MiB  required by packages that will be installed
    //      1.9 MiB  |  -  347.3 MiB  released by packages that will be removed
    out << _("Package install size change:") << endl;
    ByteCount inst_diff = _inst_size_install-_inst_size_remove;
    Table t;
    t.lineStyle( TableLineStyle::none );
    // translator: rendered as "349.1 MiB  required by packages that will be installed"
    t << ( TableRow() << ""                 <<"|"<< fmtBC( _inst_size_install ) << _("required by packages that will be installed") );
    // translator: rendered as "347.3 MiB  released by packages that will be removed"
    t << ( TableRow() << fmtBC( inst_diff ) <<"|"<< fmtBC( _inst_size_remove, "-" ) << _("released by packages that will be removed") );
    out << t; // NL terminated
  }
}

void Summary::writePackageCounts( std::ostream & out )
{
  if ( !packagesToGetAndInstall() && !packagesToRemove() )
    return;

  std::ostringstream s;
  s << endl;
  bool gotcha = false;
  unsigned count;
  KindToResPairSet::const_iterator i;

  i = _toupgrade.find( ResKind::package );
  if ( i != _toupgrade.end() && (count = i->second.size()) )
  {
    s << ( ColorContext::POSITIVE << count ) << " ";
    // translators: this text will be preceded by a number e.g. "5 packages to ..."
    s << PL_("package to upgrade", "packages to upgrade", count);
    gotcha = true;
  }
  i = _todowngrade.find(ResKind::package);
  if ( i != _todowngrade.end() && (count = i->second.size()) )
  {
    if ( gotcha )
      s << ", ";
    s << ( ColorContext::NEGATIVE << count ) << " ";
    if ( gotcha )
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << PL_("to downgrade", "to downgrade", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << PL_("package to downgrade", "packages to downgrade", count);
    gotcha = true;
  }
  i = _toinstall.find(ResKind::package);
  if (i != _toinstall.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    s << ( ColorContext::POSITIVE << count ) << " ";
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 new"
      s << PL_("new", "new", count);
    else
      // translators: this text will be preceded by a number e.g. "5 new to install"
      s << PL_("new package to install", "new packages to install", count);
    gotcha = true;
  }
  i = _toreinstall.find(ResKind::package);
  if (i != _toreinstall.end() && (count = i->second.size()) )
  {
    if (gotcha)
      s << ", ";
    s << ( ColorContext::CHANGE << count ) << " ";
    if (gotcha)
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << PL_("to reinstall", "to reinstall", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << PL_("package to reinstall", "packages to reinstall", count);
    gotcha = true;
  }
  i = _toremove.find( ResKind::package );
  if  (i != _toremove.end() && (count = i->second.size()) )
  {
    if ( gotcha )
      s << ", ";
    s << ( ColorContext::NEGATIVE << count ) << " ";
    if ( gotcha )
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << PL_("to remove", "to remove", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages to ..."
      s << PL_("package to remove", "packages to remove", count);
    gotcha = true;
  }
  i = _tochangevendor.find( ResKind::package );
  if ( i != _tochangevendor.end() && (count = i->second.size()) )
  {
    if ( gotcha )
      s << ", ";
    s << ( ColorContext::CHANGE << count ) << " ";
    if ( gotcha )
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << PL_("to change vendor", " to change vendor", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages ..."
      s << PL_("package will change vendor", "packages will change vendor", count);
    gotcha = true;
  }
  i = _tochangearch.find( ResKind::package );
  if ( i != _tochangearch.end() && (count = i->second.size()) )
  {
    if ( gotcha )
      s << ", ";
    s << ( ColorContext::CHANGE << count ) << " ";
    if ( gotcha )
      // translators: this text will be preceded by a number e.g. "5 to ..."
      s << PL_("to change arch", "to change arch", count);
    else
      // translators: this text will be preceded by a number e.g. "5 packages ..."
      s << PL_("package will change arch", "packages will change arch", count);
    gotcha = true;
  }
  i = _toinstall.find( ResKind::srcpackage );
  if ( i != _toinstall.end() && (count = i->second.size()) )
  {
    if ( gotcha )
      s << ", ";
    s << ( ColorContext::POSITIVE << count ) << " ";
    if ( gotcha )
      // translators: this text will be preceded by a number e.g. "5 new"
      s << PL_("source package", "source packages", count);
    else
      // translators: this text will be preceded by a number e.g. "5 new to install"
      s << PL_("source package to install", "source packages to install", count);
    gotcha = true;
  }
  s << "." << endl;
  mbs_write_wrapped( out, s.str(), 0, _wrap_width );
}

// --------------------------------------------------------------------------
bool Summary::showNeedRestartHint() const
{ return( _need_restart && Zypper::instance().runtimeData().plain_patch_command && !(_viewop & UPDATESTACK_ONLY) ); }

bool Summary::showNeedRebootHInt() const
{ return( _need_reboot_patch || _need_reboot_nonpatch ); }

void Summary::dumpTo( std::ostream & out )
{
  // bsc#1180077: Take care nothing is written to stdout/err if there's
  //              nothing todo. At least not if --quiet is used.
  Zypper & zypper( Zypper::instance() );

  struct SetColor
  {
    SetColor( bool force )
    : docolors( Zypper::instance().config().do_colors )
    { if (force) Zypper::instance().configNoConst().do_colors = false; }

    ~SetColor()
    { Zypper::instance().configNoConst().do_colors = docolors; }

    bool docolors;
  };
  SetColor setcolor( _force_no_color );

  _wrap_width = get_screen_width();

  // bsc#958161: While the resolver is not able to tag packages which
  // are 'recommended but not required' we suppress the output if we know
  // that all packages are actually required.
  DtorReset guard( _viewop );
  if ( getZYpp()->resolver()->onlyRequires() )
  {
    unsetViewOption( SHOW_RECOMMENDED );
    unsetViewOption( SHOW_SUGGESTED );
  }

  // bsc#993025: Suppress some information if verbosity is set to quiet
  if ( zypper.config().verbosity == Out::QUIET )
  {
    unsetViewOption( SHOW_LOCKS );
    unsetViewOption( SHOW_NOT_UPDATED );
  }

  if ( _viewop & SHOW_LOCKS )
    writeLocked( out );
  if ( _viewop & SHOW_NOT_UPDATED )
    writeNotUpdated( out );
  writeUpgraded( out );
  writeDowngraded( out );
  writeReinstalled( out );
  if ( _viewop & SHOW_RECOMMENDED )
    writeRecommended( out) ;
  if ( _viewop & SHOW_SUGGESTED )
    writeSuggested( out );
  writeChangedArch( out );
  writeChangedVendor(out);
  writeNewlyInstalled( out );
  writeRemoved( out );
  writeAutoResolved( out );
  if ( _viewop & SHOW_UNSUPPORTED )
  {
    writeSupportUnknown( out );
    writeSupportUnsupported( out );
    writeSupportNeedACC( out );
    writeSupportSuperseded( out );
  }
  writeRebootNeeded( out );
  writePackageCounts( out );
  writeDownloadAndInstalledSizeSummary( out );
  if ( showNeedRestartHint() )
  {
    // patch command (auto)restricted to update stack patches
    Zypper::instance().out().notePar( 4, _("Package manager restart required. (Run this command once again after the update stack got updated)") );
  }
  if ( showNeedRebootHInt() )
  {
    if ( _download_only || _dryRun ) {
      std::string_view opt { _dryRun ? "--dry-run" : "--download-only" };
      // translator: %1% is an option string like  "--dry-run"
      Zypper::instance().out().notePar( 4, str::Format( _("%1% is set, otherwise a system reboot would be required.") ) % opt );
    } else {
      Zypper::instance().out().notePar( 4, _("System reboot required.") );
    }
  }

  if ( !_ctc.empty() )
  {
    str::Str s;
    for ( const auto & str : _ctc )
    {
      s << endl << str;
    }
    // translator: Printed after the summary but before the prompt to start the installation.
    // Followed by some explanatory text telling it might be a good idea NOT to continue:
    //
    // # zypper up
    // ...
    //    Consider to cancel:
    //    Product 'opennSUSE Tumbleweed' requires to be upgraded by calling 'zypper dup'!
    // Continue? [y/n/...? shows all options] (y):
    Zypper::instance().out().taggedPar( 4, MSG_WARNINGString(_("Consider to cancel:") ), s );
  }
}

// --------------------------------------------------------------------------

void Summary::writeXmlResolvableList( std::ostream & out, const KindToResPairSet & resolvables )
{
  for_( it, resolvables.begin(), resolvables.end() )
  {
    for_( pairit, it->second.begin(), it->second.end() )
    {
      ResObject::constPtr res( pairit->second );
      ResObject::constPtr rold( pairit->first );

      out << "<solvable";
      out << " type=\"" << res->kind() << "\"";
      out << " name=\"" << res->name() << "\"";
      out << " edition=\"" << res->edition() << "\"";
      out << " arch=\"" << res->arch() << "\"";
      out << " repository=\"" << res->repoInfo().alias() << "\"";
      if ( rold )
      {
        out << " edition-old=\"" << rold->edition() << "\"";
        out << " arch-old=\"" << rold->arch() << "\"";
      }
      {
        const std::string & text( res->summary() );
        if ( !text.empty() )
          out << " summary=\"" << xml::escape(text) << "\"";
      }
      {
        const std::string & text( res->description() );
        if ( !text.empty() )
          out << ">\n" << "<description>" << xml::escape( text ) << "</description>" << "</solvable>" << endl;
        else
          out << "/>" << endl;
      }
    }
  }
}

// --------------------------------------------------------------------------

void Summary::dumpAsXmlTo( std::ostream & out )
{
  unsigned pkgchanged = _inst_pkg_total;
  const auto & iter = _toremove.find( ResKind::package );
  if ( iter != _toremove.end() )
    pkgchanged += iter->second.size();
  zypp::ByteCount _inst_size_change = _inst_size_install - _inst_size_remove;

  out << "<install-summary";
  out << " download-size=\"" << ((ByteCount::SizeType)_todownload) << "\"";
  out << " space-usage-diff=\"" << ((ByteCount::SizeType)_inst_size_change) << "\"";
  out << " space-usage-installed=\"" << ((ByteCount::SizeType)_inst_size_install) << "\"";
  out << " space-usage-removed=\"" << ((ByteCount::SizeType)_inst_size_remove) << "\"";
  out << " packages-to-change=\"" << pkgchanged << "\"";	// bsc#1102429: CaaSP requires it to detect 'nothing to do'
  out << " need-restart=\"" << showNeedRestartHint() << "\"";	// bsc#1188435: show need reboot/restart hints
  out << " need-reboot=\"" << showNeedRebootHInt() << "\"";	// -"-
  out << ">" << endl;

  if ( !_toupgrade.empty() )
  {
    out << "<to-upgrade>" << endl;
    writeXmlResolvableList( out, _toupgrade );
    out << "</to-upgrade>" << endl;
  }

  if ( !_todowngrade.empty() )
  {
    out << "<to-downgrade>" << endl;
    writeXmlResolvableList( out, _todowngrade );
    out << "</to-downgrade>" << endl;
  }

  if ( !_toinstall.empty() )
  {
    out << "<to-install>" << endl;
    writeXmlResolvableList( out, _toinstall );
    out << "</to-install>" << endl;
  }

  if ( !_toreinstall.empty() )
  {
    out << "<to-reinstall>" << endl;
    writeXmlResolvableList( out, _toreinstall );
    out << "</to-reinstall>" << endl;
  }

  if ( !_toremove.empty() )
  {
    out << "<to-remove>" << endl;
    writeXmlResolvableList( out, _toremove );
    out << "</to-remove>" << endl;
  }

  if ( !_tochangearch.empty() )
  {
    out << "<to-change-arch>" << endl;
    writeXmlResolvableList( out, _tochangearch );
    out << "</to-change-arch>" << endl;
  }

  if ( !_tochangevendor.empty() )
  {
    out << "<to-change-vendor>" << endl;
    writeXmlResolvableList( out, _tochangevendor );
    out << "</to-change-vendor>" << endl;
  }

  if ( _viewop & SHOW_UNSUPPORTED && !( _supportUnknown.empty() && _supportUnsupported.empty() ) )
  {
    out << "<_unsupported>" << endl;
    writeXmlResolvableList( out, _supportUnknown );
    writeXmlResolvableList( out, _supportUnsupported );
    out << "</_unsupported>" << endl;
  }

  out << "</install-summary>" << endl;
}
