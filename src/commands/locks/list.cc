/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "list.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Locks.h>

#include "output/Out.h"
#include "Table.h"
#include "Zypper.h"
#include "main.h"

#include "utils/flags/zyppflags.h"
#include "utils/flags/flagtypes.h"

using namespace zypp;

///////////////////////////////////////////////////////////////////
namespace out
{
  inline std::ostream & dumpAsXmlOn( std::ostream & str, const Rel & op_r, const Edition & ed_r )
  {
    xmlout::Node n { str, "range", xmlout::Node::optionalContent,
      { { "flag", op_r } }
    };
    if ( op_r != Rel::ANY && op_r != Rel::NONE )
      n.addAttr( {
        { "epoch",   ed_r.epoch() },
        { "version", ed_r.version() },
        { "release", ed_r.release() },
      } );
    return str;
  }

  struct LocksTableFormater : public TableFormater
  {
  private:
    /** LESS compare for MatchDetails */
    struct MatchDetailCompare
    {
      bool operator()( const sat::Solvable & lhs, const sat::Solvable & rhs ) const
      { return( doComapre( lhs, rhs ) < 0 ); }

      int doComapre( const sat::Solvable & lhs, const sat::Solvable & rhs ) const
      {
        // do N(<) A(>) VR(>)
        int res = sat::compareByN( lhs, rhs );									// ascending  l<r
        if ( res == 0 ) res = rhs.arch().compare( lhs.arch() );							// descending r<l
        if ( res == 0 ) res = rhs.edition().compare( lhs.edition() );						// descending r<l
        if ( res == 0 ) res = lhs.repository().asUserString().compare( rhs.repository().asUserString() );	// ascending  l<r
        return res;
      }
    };
    /** Ordered MatchDetails */
    typedef std::set<sat::Solvable,MatchDetailCompare> MatchDetails;

    /** MatchDetail representation */
    struct MatchDetailFormater
    {
      std::string xmlListElement( const sat::Solvable & match_r ) const
      {
        str::Str str;
        {
          xmlout::Node lock( str.stream(), "match", {
            { "kind",		match_r.kind()	},
            { "name",		match_r.name()	},
            { "edition",	match_r.edition()	},
            { "arch",		match_r.arch()	},
            { "installed",	match_r.isSystem()	},
            { "repo",		match_r.repository().alias()	},
          } );
        }
        return str;
      }
    };

  public:
    std::string xmlListElement( const PoolQuery & q_r ) const
    {
      str::Str str;
      // <lock>
      {
        ++_i;
        xmlout::Node lock( str.stream(), "lock", { { "number", _i } } );

        // <name> (solvable_name)
        for ( const std::string & val : q_r.attribute( sat::SolvAttr::name ) )
        { *xmlout::Node( *lock, "name" ) << val; }
        // <name> (query_string)
        for ( const std::string & val : q_r.strings() )
        { *xmlout::Node( *lock, "name" ) << val; }

        // <type>
        for ( const ResKind & kind : q_r.kinds() )
        { *xmlout::Node( *lock, "type" ) << kind; }

        // <repo>
        for ( const std::string & repo : q_r.repos() )
        { *xmlout::Node( *lock, "repo" ) << repo; }

        // <range>
        if ( q_r.editionRel() != Rel::ANY )
        {
          dumpAsXmlOn( *lock, q_r.editionRel(), q_r.edition() );
        }

        if ( _withMatches )
        {
          // <matches>
          xmlout::Node matches( *lock, "matches", xmlout::Node::optionalContent, { { "size", q_r.size() } } );
          if ( _withSolvables && !q_r.empty() )
          {
            MatchDetails d;
            getLockDetails( q_r, d );
            xmlWriteContainer( *matches, d, MatchDetailFormater() );
          }
        }
      }
      return str;
    }

    TableHeader header() const
    {
      TableHeader th;
      th << "#" << N_("Name");
      if ( _withMatches )
        th << N_("Matches");
      th << N_("Type") << N_("Repository") << N_("Comment");
      return th;
    }

    TableRow row( const PoolQuery & q_r ) const
    {
      TableRow tr;

      // #
      ++_i;
      tr << str::numstring( _i );

      // Name
      const PoolQuery::StrContainer & nameStings( q_r.attribute( sat::SolvAttr::name ) );
      const PoolQuery::StrContainer & globalStrings( q_r.strings() );

      if ( nameStings.size() + globalStrings.size() > 1 )
        // translators: locks table value
        tr << _("(multiple)");
      else if ( nameStings.empty() && globalStrings.empty() )
        // translators: locks table value
        tr << _("(any)");
      else
        tr << makeNameString( nameStings.empty() ? *globalStrings.begin() : *nameStings.begin(), q_r.editionRel(), q_r.edition() );

      // opt Matches
      if ( _withMatches )
        tr << q_r.size();

      // Type
      std::set<std::string> strings;
      for ( const ResKind & kind : q_r.kinds() )
        strings.insert( kind.asString() );
      tr << get_string_for_table( strings );

      // Repository
      strings.clear();
      copy( q_r.repos().begin(), q_r.repos().end(), inserter(strings, strings.end()) );
      tr << get_string_for_table( strings );

      // Comment
      tr << q_r.comment();

      // opt Solvables as detail
      if ( _withSolvables && !q_r.empty() )
      {
        MatchDetails i;
        MatchDetails a;
        getLockDetails( q_r, i, a );

        PropertyTable p;
        {
          std::vector<std::string> names;
          names.reserve( std::max( i.size(), a.size() ) );
          if ( ! i.empty() )
          {
            //names.clear();
            for ( const auto & solv : i )
            { names.push_back( solv.asUserString() ); }
            // translators: property name; short; used like "Name: value"
            p.add( _("Keep installed"), names );
          }
          if ( ! a.empty() )
          {
            names.clear();
            for ( const auto & solv : a )
            { names.push_back( solv.asUserString() ); }
            // translators: property name; short; used like "Name: value"
            p.add( _("Do not install"), names );
          }
        }
        tr.addDetail( str::Str() << p );
      }
      return tr;
    }

    LocksTableFormater( bool withSolvables, bool withMatches )
    : _withSolvables( withSolvables )
    , _withMatches( _withSolvables || withMatches )
    {}

  private:
    static std::string makeNameString( const std::string & name_r, const Rel & op_r, const Edition & edition_r )
    {
      str::Str ret;
      ret << name_r;
      if ( op_r != Rel::ANY )
        ret << " " << op_r << " " << edition_r;
      return ret;
    }

    static std::string get_string_for_table( const std::set<std::string> & attrvals_r )
    {
      std::string ret;
      if ( attrvals_r.empty() )
        ret = _("(any)");
      else if ( attrvals_r.size() > 1 )
        ret = _("(multiple)");
      else
        ret = *attrvals_r.begin();
      return ret;
    }

    static void getLockDetails( const PoolQuery & q_r, MatchDetails & i_r, MatchDetails & a_r )
    { for ( const auto & solv : q_r ) { (solv.isSystem()?i_r:a_r).insert( solv ); } }

    static void getLockDetails( const PoolQuery & q_r, MatchDetails & d_r )
    { getLockDetails( q_r, d_r, d_r ); }

  private:
    bool _withSolvables	:1;	//< include match details (implies _withMatches)
    bool _withMatches	:1;	//< include number of matches
    mutable unsigned _i = 0;	//< Lock Number
  };
} // namespace out
///////////////////////////////////////////////////////////////////

ListLocksCmd::ListLocksCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("locks (ll) [OPTIONS]"),
    _("List current package locks.")
    )
{}

std::string ListLocksCmd::description() const
{
  return summary();
}


int ListLocksCmd::systemSetup(Zypper &zypper)
{
  SetupSystemFlags flags = DisableAll;
  if ( _matches || _solvables )
    flags.setFlag( DefaultSetup );
  return defaultSystemSetup( zypper, flags );
}

void ListLocksCmd::doReset()
{
  _matches = false;
  _solvables = false;
}

ZyppFlags::CommandGroup ListLocksCmd::cmdOptions() const
{
  return {{
    {
      { "matches", 'm', ZyppFlags::NoArgument, ZyppFlags::BoolType( const_cast<bool *>(&_matches), ZyppFlags::StoreTrue, _matches), _("Show the number of resolvables matched by each lock.") },
      { "solvables", 's', ZyppFlags::NoArgument, ZyppFlags::BoolType( const_cast<bool *>(&_solvables), ZyppFlags::StoreTrue, _solvables), _("List the resolvables matched by each lock.")}
    }
  }};
}

int ListLocksCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs)
{
  Locks & locks( Locks::instance() );
  try
  {
    locks.read( Pathname::assertprefix( zypper.config().root_dir, ZConfig::instance().locksFile() ) );
  }
  catch( const Exception & e )
  {
    zypper.out().error( str::Format( _("Error reading the locks file:") ) % e);
    return ZYPPER_EXIT_ERR_ZYPP;
  }

  // show result
  Out & out( zypper.out() );
  out.gap();
  out.table( "locks", locks.empty() ? _("There are no package locks defined.") : "",
             locks, out::LocksTableFormater( _solvables, _matches ) );
  out.gap();

  return 0;
}
