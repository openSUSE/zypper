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

struct LocaleWithState {
  enum State {
    NotRequested,
    Requested,
    Fallback
  };

  Locale locale;
  State  state;

  bool operator<(const LocaleWithState &other ) const {
    return this->locale < other.locale;
  }
};

using LocaleWithStateSet = std::set<LocaleWithState>;

static std::string isRequestedState( LocaleWithState::State state )
{
  switch ( state ) {
    case LocaleWithState::Requested:
      return _("Requested");
    case LocaleWithState::NotRequested:
      return _("No");
    case LocaleWithState::Fallback:
      return _("Fallback");
  };
  return "";
}

static bool isRequestedFallbackLocale ( const Locale &locale )
{
  for ( const Locale &loc : God->pool().getRequestedLocales() ) {
    Locale locFallback = loc.fallback();
    while ( locFallback != Locale::noCode ) {
      if ( locFallback == locale  ) {
        return true;
      }
      locFallback = locFallback.fallback();
    }
  }
  return false;
}

static void printLocaleList( Zypper & zypper, const LocaleWithStateSet &locales )
{
  Table tbl;
  // header
  TableHeader th;

  // translators: header of table column - the language code, e.g. en_US
  th << _("Code");
  // translators: header of table column - the language, e.g. English (United States)
  th << _("Language");
  // translators: header of table column - is the language requested? (Yes/No)
  th << _("Requested");

  tbl << th;

  for_( it, locales.begin(), locales.end() )
  {
    TableRow tr(3);

    tr << (*it).locale.code();
    tr << (*it).locale.name();
    tr << isRequestedState(it->state);
    tbl << tr;
  }

  tbl.sort(1);

  cout << tbl;
}

#if 0
static void printXmlLocaleList( Zypper & zypper, const zypp::LocaleSet &locales )
{

}
#endif

static void printLocalePackages( Zypper & zypper, const zypp::sat::LocaleSupport & myLocale )
{
  Table tbl;
  TableHeader th;

  // translators: header of table column - S is for 'Status' (package installed or not)
  th << _("S");
  // translators: header of table column - the name of the package
  th << _("Name");
  // translators: header of table column - the package summary
  th << _("Description");

  tbl << th;

  for_( it, myLocale.selectableBegin(), myLocale.selectableEnd() )
  {
    TableRow tr(3);
    zypp::ui::Status status = (*it)->status();

    if ( status == zypp::ui::S_KeepInstalled || status == zypp::ui::S_Protected )
      tr << "i";
    else
      tr << " ";
    tr << (*it)->name();
    tr << (*it)->theObj()->summary();

    tbl << tr;
  }

  tbl.sort(1);

  cout << tbl;
}

static zypp::LocaleSet relevantLocales( const std::vector<std::string> &localeArgs, bool relaxed )
{
  const zypp::LocaleSet & availableLocales( God->pool().getAvailableLocales() );

  zypp::LocaleSet resultSet;

  for_( it, availableLocales.begin(), availableLocales.end() )
  {
    if ( localeArgs.empty() )
    {
      // without argument take only requested locales
      if ( God->pool().isRequestedLocale(*it) )
      {
        resultSet.insert(*it);
      }
    }
    else
    {
      // take given locales (if available)
      for_( loc, localeArgs.begin(), localeArgs.end() )
      {
        if ( relaxed ) {

          size_t strSize = loc->size();

          //match all starting with the given country code but exclude the fallback one
          bool underscoreMatch = str::endsWith( *loc, "_" );

          //match all starting with the given country code including the fallback one
          bool asteriskMatch = str::endsWith( *loc, "*" );

          if ( asteriskMatch || underscoreMatch )  {
            if ( strSize < 2 ) {
              WAR << "Ignoring too short argument " << *loc << endl;
              continue;
            }
            std::string langCode = loc->substr( 0, strSize - 1 );
            if ( it->language().code() == langCode ) {
              if ( underscoreMatch && it->country() != CountryCode::noCode )
                resultSet.insert(*it);
              else if ( asteriskMatch )
                resultSet.insert(*it);
            }
            continue;
          }
        }

        //exact match
        if ( *loc == (*it).code().c_str() ) {
          resultSet.insert(*it);
        }
      }
    }
  }

  return resultSet;
}

/**
 * Adds fallbacks if not already in the set and flags each entry with
 * the state it is currently in
 */
static LocaleWithStateSet addMissingInfo ( const zypp::LocaleSet &locales, bool insertMissingFallbacks )
{
  LocaleWithStateSet result;

  auto insertOrOverwrite = [&result]( LocaleWithState::State state, const Locale &newElem ) {
    auto inserted = result.insert( LocaleWithState{ newElem, state } );
    if ( !inserted.second && inserted.first->state != state) {

      // if we were not able to insert the value, there is already a existing element
      // try to override it, but first check if that is allowed:
      // REQ -> ( REQ, NREQ, FB )
      // NR  -> ()
      // FB  -> ( NREQ, FB )

      const LocaleWithState &existing = *inserted.first;
      bool canOverride =   state == LocaleWithState::Requested || //requested always wins
                         ( state == LocaleWithState::Fallback && existing.state == LocaleWithState::NotRequested );

      if ( canOverride ) {
        result.erase( inserted.first );
        if ( ! result.insert( LocaleWithState{ newElem, state } ).second ) {
          WAR << "Unable to insert locale: "<< newElem << " into the result set " << endl;
        }
      }
    }
  };

  for ( const Locale &loc : locales ) {

    LocaleWithState::State state = LocaleWithState::NotRequested;
    if ( God->pool().isRequestedLocale( loc ) ) {
      state = LocaleWithState::Requested;
    } else if ( isRequestedFallbackLocale( loc ) ) {
      state = LocaleWithState::Fallback;
    }

    insertOrOverwrite( state, loc );

    if ( insertMissingFallbacks ) {
      Locale locFallback = loc.fallback();
      while ( locFallback != Locale::noCode ) {
        insertOrOverwrite( isRequestedFallbackLocale( locFallback ) ? LocaleWithState::Fallback : LocaleWithState::NotRequested, locFallback );
        locFallback = locFallback.fallback();
      }
    }
  }

  return result;
}

void listLocales( Zypper & zypper, const std::vector<std::string> &localeArgs, bool showAll )
{
  zypp::LocaleSet locales;

  if ( showAll ) {
    const auto &avail = God->pool().getAvailableLocales();
    const auto &req = God->pool().getRequestedLocales();
    std::merge( avail.begin(), avail.end(), req.begin(), req.end(), std::inserter( locales, locales.begin() ) );
  } else {
    locales = relevantLocales( localeArgs, true );
  }

#if 0
  // print xml output
  if ( zypper.out().type() == Out::TYPE_XML )
    printXmlLocaleList(zypper, locales);
  else
#endif
  printLocaleList( zypper, addMissingInfo( locales, showAll || localeArgs.size() == 0 ) );
}

void localePackages( Zypper &zypper, const std::vector<std::string> &localeArgs, bool showAll )
{
   zypp::LocaleSet locales;

   if ( showAll ) {
     const auto &avail = God->pool().getAvailableLocales();
     const auto &req = God->pool().getRequestedLocales();
     std::merge( avail.begin(), avail.end(), req.begin(), req.end(), std::inserter( locales, locales.begin() ) );
   } else {
    locales = relevantLocales( localeArgs, true );
   }

  LocaleWithStateSet locs = addMissingInfo( locales, showAll || localeArgs.size() == 0  );

  std::list<LocaleWithState> sortedLocales ( locs.begin(), locs.end() );
  sortedLocales.sort( [] ( const LocaleWithState &l, const LocaleWithState &r ) {
    return l.locale.code() < r.locale.code();
  });

  for_( it, sortedLocales.begin(), sortedLocales.end() )
  {
    const zypp::sat::LocaleSupport & myLocale((*it).locale);
    cout << endl;
    zypper.out().info( str::form( _("Packages for %s (locale '%s', requested: %s):"),
      it->locale.name().c_str(), it->locale.code().c_str(), isRequestedState(it->state).c_str() ) );
    cout << endl;
    printLocalePackages( zypper, myLocale );
  }
}

void addLocales( Zypper &zypper, const std::vector<std::string> &localeArgs_r, bool packages, std::map<std::string, bool> *result )
{
  const zypp::LocaleSet locales = relevantLocales( localeArgs_r, false );

  for_( it, locales.begin(), locales.end() ) {
    bool success = false;

    if ( !God->pool().isRequestedLocale(*it) ) {
      success = God->pool().addRequestedLocale( *it );
      if ( success ) {
        zypper.out().info( str::form( _("Added locale: %s"), (*it).code().c_str() ) );
        if ( result ) ( *result )[(*it).code().c_str()] = true;
      } else {
        zypper.out().error( str::form( _("ERROR: cannot add %s"), (*it).code().c_str() ) );
        if ( result ) ( *result )[(*it).code().c_str()] = false;
      }
    } else {
      zypper.out().info( str::form( _(" %s is already requested."), (*it).code().c_str() ) );
      if ( result ) ( *result )[(*it).code().c_str()] = false;
    }
  }

  if ( packages ) {
    solve_and_commit( zypper, Summary::DEFAULT, DownloadMode::DownloadDefault );
  } else {
    God->commit( ZYppCommitPolicy() );
  }
}

void removeLocales( Zypper &zypper, const std::vector<std::string> &localeArgs, bool packages, std::map<std::string, bool> *result )
{
  for_( it, localeArgs.begin(), localeArgs.end() ) {
    bool success = false;

    zypp::Locale loc( *it );
    if ( God->pool().isRequestedLocale( loc ) ) {
      success = God->pool().eraseRequestedLocale( loc );
      if ( success ) {
        zypper.out().info( str::form( _("Removed locale: %s"), (*it).c_str() ) );
        ( *result )[(*it)] = true;
      } else {
        zypper.out().error( str::form( _("ERROR: cannot remove %s"), (*it).c_str() ) ) ;
        ( *result )[(*it)] = false;
      }
    } else {
      zypper.out().info( str::form( _("%s was not requested."), (*it).c_str() ) );
      ( *result )[(*it)] = false;
    }
  }

  if ( packages ) {
    solve_and_commit( zypper, Summary::DEFAULT, DownloadMode::DownloadDefault );
  } else {
    God->commit( ZYppCommitPolicy() );
  }
}
