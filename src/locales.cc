/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <iterator>
#include <list>

#include <zypp/ZYpp.h>
#include <zypp/base/Logger.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/String.h>
#include <zypp/base/Flags.h>
#include <zypp/ui/Selectable.h>
#include <zypp/base/Regex.h>
#include <zypp/sat/LocaleSupport.h>

#include "output/Out.h"
#include "main.h"
#include "getopt.h"
#include "Table.h"
#include "utils/messages.h"
#include "utils/misc.h" // for xml_encode
#include "locales.h"
#include "solve-commit.h"

#include "Zypper.h"

extern ZYpp::Ptr God;


/////////////////////////////////////////////////////////////////
namespace
{
  struct LocaleState
  {
    enum State {
      NotRequested = 0,    ///< Not relevant
      Requested    = 1<<1, ///< A requested locale (supersedes also being a Fallback)
      Fallback     = 1<<2, ///< Fallback of a requested locale (may also be requested)
      Available    = 1<<3, ///< We saw at lest one package supporting the locale
    };

    LocaleState()
    {}

    LocaleState( State state_r )
    : _state { state_r }
    {}

    bool isNone() const		{ return !_state; }
    bool isRequested() const	{ return _state & Requested; }
    bool isFallback() const	{ return _state & Fallback; }
    bool isAvailable() const	{ return _state & Available; }

    void tagRequested()		{ if ( !isRequested() ) _state |= Requested; }
    void tagFallback()		{ if ( !isFallback() ) _state |= Fallback; }
    void tagAvailable()		{ if ( !isAvailable() ) _state |= Available; }

    std::string asString() const
    {
      std::string ret;
      if ( isRequested() )
        ret = _("Requested");
      else if ( isFallback() )
        ret = _("Fallback");
      else {
        ret = _("No");
        if ( !isAvailable() )
          ret += " (n/a)";
      }
      return ret;
    }

  private:
    unsigned _state = 0;
  };

  using LocaleStateMap = std::map<Locale,LocaleState>;

  /** 	lloc helper building the set of locales to show */
  inline LocaleStateMap argsToLocaleSet( const std::vector<std::string> & localeArgs_r, bool showAll_r )
  {
    LocaleStateMap ret;
    bool mustFixState = true;	// If locales a added in No state and later checked if Requested/Fallback

    if ( showAll_r )
    {
      for ( Locale rloc : God->target()->requestedLocales() )
        ret[rloc].tagRequested();
      for ( const auto & avloc : God->pool().getAvailableLocales() )
        ret[avloc].tagAvailable();	// Requested/Fallback state will be adjusted later
    }
    else if ( ! localeArgs_r.empty() )
    {
      // Search patterns are matched against available locales.
      // Exact locales are added even if not in available locales.
      const auto & av { God->pool().getAvailableLocales() };
      for ( const auto & larg : localeArgs_r )
      {
        std::string::size_type suffix = 0;
        bool includeNoCountryCode = false;
        if ( str::endsWith( larg, "_" ) )	// Suffix _ (_*) require a (not empty) country code
          suffix = 1;
        else if ( str::endsWith( larg, "_*" ) )	// --"--  _* as convenience for the above
          suffix = 2;
        else if ( str::endsWith( larg, "*" ) )	// Suffix * (except _*) also matches an empty country code
        {
          suffix = 1;
          includeNoCountryCode = true;
        }

        if ( suffix )
        {
          const std::string & stem { larg.substr( 0, larg.size() - suffix ) };
          for ( const auto & avloc : av )
          {
            if ( avloc.language().code() == stem && ( includeNoCountryCode || avloc.country().code() != CountryCode::noCode ) )
              ret[avloc].tagAvailable();	// Requested/Fallback state will be adjusted later
          }
        }
        else	// Exact locale arg
        {
          Locale loc { larg };
          if ( av.count( loc ) )	// Requested/Fallback state will be adjusted later
            ret[loc].tagAvailable();
          else
            ret[loc];
        }
      }
    }
    else // no args/all, so we show all requested
    {
      // Taking the requested locales from Target works even
      // if the target is just initialized but nor loaded.
      for ( Locale rloc : God->target()->requestedLocales() )
      {
        ret[rloc].tagRequested();
        while ( (rloc = rloc.fallback()) )
          ret[rloc].tagFallback();
      }
      mustFixState = false;	// added with the right status
    }

    if ( mustFixState )
    {
      // Now adjust Requested/Fallback state for all collected locales
      // Avaialble state should have been set before.
      for ( Locale rloc : God->target()->requestedLocales() )
      {
        auto it { ret.find( rloc ) };
        if ( it != ret.end() )
          it->second.tagRequested();
        while ( (rloc = rloc.fallback()) )
        {
          it = ret.find( rloc );
          if ( it != ret.end() )
            it->second.tagFallback();
        }
      }
    }
    return ret;
  }

  void printLocaleList( Zypper & zypper, const LocaleStateMap & locales_r )
  {
    Table tbl;
    tbl << ( TableHeader()
    // translators: header of table column - the language code, e.g. en_US
    << N_("Code")
    // translators: header of table column - the language, e.g. English (United States)
    << N_("Language")
    // translators: header of table column - is the language requested? (Yes/No)
    << N_("Requested")
    );

    for ( const auto & el : locales_r )
    {
      tbl << ( TableRow(3, el.second.isRequested() ? ColorContext::DEFAULT : ColorContext::LOWLIGHT )
      << el.first.code()
      << el.first.name()
      << el.second.asString()
      );
    }

    tbl.sort( 0 );
    cout << tbl;
  }

  void printLocalePackages( Zypper & zypper, const zypp::sat::LocaleSupport & myLocale )
  {
    Table tbl;
    tbl << ( TableHeader()
    // translators: header of table column - S is for 'Status' (package installed or not)
    << N_("S")
    // translators: header of table column - the name of the package
    << N_("Name")
    // translators: header of table column - the package summary
    << N_("Description")
    );

    for_( it, myLocale.selectableBegin(), myLocale.selectableEnd() )
    {
      TableRow tr(3);
      tr << computeStatusIndicator( *(*it) );
      tr << (*it)->name();
      tr << (*it)->theObj()->summary();

      tbl << tr;
    }

    tbl.sort(1);
    cout << tbl;
  }

#if 0
  void printXmlLocaleList( Zypper & zypper, const zypp::LocaleSet &locales )
  {
  }
#endif
} // namespace
/////////////////////////////////////////////////////////////////



void listLocales( Zypper & zypper, const std::vector<std::string> &localeArgs, bool showAll )
{
  printLocaleList( zypper, argsToLocaleSet( localeArgs, showAll ) );
}

void localePackages( Zypper &zypper, const std::vector<std::string> &localeArgs, bool showAll )
{
  for ( const auto & el : argsToLocaleSet( localeArgs, showAll ) )
  {
    zypper.out().gap();
    zypper.out().info( str::form( _("Packages for %s (locale '%s', requested: %s):"),
                                  el.first.name().c_str(), el.first.code().c_str(), el.second.asString().c_str() ) );
    zypper.out().gap();
    printLocalePackages( zypper, el.first );
  }
}

void addLocales( Zypper &zypper, const std::vector<std::string> &localeArgs_r, bool packages, std::map<std::string, bool> *result )
{
  for ( const auto & arg : std::set<std::string>( localeArgs_r.begin(), localeArgs_r.end() ) )
  {
    if ( God->pool().addRequestedLocale( Locale(arg) ) )
    {
      zypper.out().info( str::Format(_("Added locale: %s") ) % arg );
      if ( result ) (*result)[arg] = true;
    }
    else
    {
      zypper.out().info( str::Format(_(" %s is already requested.") ) % arg );
      if ( result ) (*result)[arg] = false;
    }
  }

  if ( packages ) {
    solve_and_commit(
      zypper,
      SolveAndCommitPolicy()
        .summaryOptions( Summary::DEFAULT )
        .forceCommit( true )
    );
  } else {
    God->commit( ZYppCommitPolicy() );
  }
}

void removeLocales( Zypper &zypper, const std::vector<std::string> &localeArgs_r, bool packages, std::map<std::string, bool> *result )
{
  for ( const auto & arg : std::set<std::string>( localeArgs_r.begin(), localeArgs_r.end() ) )
  {
    if ( God->pool().eraseRequestedLocale( Locale(arg) ) )
    {
      zypper.out().info( str::Format(_("Removed locale: %s") ) % arg );
      if ( result ) (*result)[arg] = true;
    }
    else
    {
      zypper.out().info( str::Format(_("%s was not requested.") ) % arg );
      if ( result ) (*result)[arg] = false;
    }
  }

  if ( packages ) {
    solve_and_commit(
      zypper,
      SolveAndCommitPolicy()
        .summaryOptions( Summary::DEFAULT )
        .forceCommit( true )
      );
  } else {
    God->commit( ZYppCommitPolicy() );
  }
}
