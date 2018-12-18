#include <iostream>
#include <boost/lexical_cast.hpp>

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Locks.h>

#include "output/Out.h"
#include "main.h"
#include "Table.h"
#include "utils/misc.h"
#include "locks.h"
#include "repos.h"

using namespace zypp;

// OLD STYLE VERSIONED LOCKS:
//	solvable_name: kernel
//	version: > 1
//
// NEW STYLE VERSIONED LOCKS:
//	complex: AttrMatchData solvable:name kernel C SolvableRange\ >\ 1\ \"\"
//   or
//	solvable_name: kernel > 1
//
// Semantically equivalent as locks, but due to the different syntax
// the complex lock is wrongly handled in list. Different syntax also
// may prevent removing locks (old and new style locks are not ==).
//
// bsc#1112911: Unfortunately all styles are found in real-life locks-files.
// libzypp will try to make sure, when parsing the locks-file, that only
// OLD STYLE queries are generated. They should work for list and remove.
#undef	ENABLE_NEW_STYLE_VERSIONED_LOCKS

///////////////////////////////////////////////////////////////////
namespace locks
{
  using namespace zypp;
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline void addNameDependency( PoolQuery & q_r, const std::string & arg_r )
    {
      CapDetail d { Capability(arg_r) };
      if ( d.isVersioned() )
      {
#ifdef ENABLE_NEW_STYLE_VERSIONED_LOCKS
	q_r.addDependency( sat::SolvAttr::name, d.name().asString(), d.op(), d.ed() );
#else
	q_r.addAttribute( sat::SolvAttr::name, d.name().asString() );
	q_r.setEdition( d.ed(), d.op() );
#endif
      }
      else
      {
#ifdef ENABLE_NEW_STYLE_VERSIONED_LOCKS
	q_r.addDependency( sat::SolvAttr::name, arg_r );
#else
	q_r.addAttribute( sat::SolvAttr::name, arg_r );
#endif
      }
    }
  }
  ///////////////////////////////////////////////////////////////////

  PoolQuery arg2query( Zypper & zypper, const std::string & arg_r, const std::set<ResKind> & kinds_r )
  {
    // Try to stay with the syntax the serialized query (AKA lock) generates.
    //     type: package
    //     match_type: glob
    //     case_sensitive: on
    //     solvable_name: kernel

    PoolQuery q;
    q.setMatchGlob();
    q.setCaseSensitive();

    parsed_opts::const_iterator itr = copts.find( "repo" );
    if ( itr != copts.end() )
    {
      const std::list<std::string> & repos( itr->second );
      for_( it, repos.begin(), repos.end() )
      {
	RepoInfo info;
	if ( match_repo( zypper, *it, &info ) )
	  q.addRepo( info.alias() );
	else //TODO some error handling
	  WAR << "unknown repository" << *it << endl;
      }
    }

    if ( kinds_r.empty() || ResKind::explicitBuiltin( arg_r ) ) // derive it from the name
    {
      sat::Solvable::SplitIdent split { arg_r };
      addNameDependency( q, split.name().asString() );
      q.addKind( split.kind() );
    }
    else
    {
      addNameDependency( q, arg_r );
      for_( it, kinds_r.begin(), kinds_r.end() )
	q.addKind( *it );
    }

    return q;
  }

} // namespace locks
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace out
{
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
	int res = sat::compareByN( lhs, rhs );			// ascending  l<r
	if ( res == 0 )
	  res = rhs.arch().compare( lhs.arch() );		// descending r<l
	  if ( res == 0 )
	    res = rhs.edition().compare( lhs.edition() );	// descending r<l
	    if ( res == 0 )
	      res = lhs.repository().asUserString().compare( rhs.repository().asUserString() );	// ascending  l<r
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
      th << "#" << _("Name");
      if ( _withMatches )
	th << _("Matches");
      th << _("Type") << _("Repository");
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
      else if ( nameStings.empty() )
        tr << *globalStrings.begin();
      else
        tr << *nameStings.begin();

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

    LocksTableFormater( shared_ptr<ListLocksOptions> listLocksOptions_r )
    : _withSolvables( listLocksOptions_r->_withSolvables )
    , _withMatches( _withSolvables || listLocksOptions_r->_withMatches )
    {}

  private:
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
} //namespace out
///////////////////////////////////////////////////////////////////

void list_locks( Zypper & zypper )
{
  Locks & locks( Locks::instance() );
  try
  {
    locks.read( Pathname::assertprefix( zypper.globalOpts().root_dir, ZConfig::instance().locksFile() ) );
  }
  catch( const Exception & e )
  {
    throw( Out::Error( ZYPPER_EXIT_ERR_ZYPP, _("Error reading the locks file:"), e ) );
  }

  // show result
  Out & out( zypper.out() );
  out.gap();
  out.table( "locks", locks.empty() ? _("There are no package locks defined.") : "",
	     locks, out::LocksTableFormater( zypper.commandOptionsOrDefaultAs<ListLocksOptions>() ) );
  out.gap();
}

template <typename Target, typename Source>
void safe_lexical_cast (Source s, Target &tr) {
  try {
    tr = boost::lexical_cast<Target> (s);
  }
  catch (boost::bad_lexical_cast &) {
  }
}

void add_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds)
{
  try
  {
    Locks & locks = Locks::instance();
    locks.read(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));
    Locks::size_type start = locks.size();
    for_(it,args.begin(),args.end())
    {
      locks.addLock( locks::arg2query( zypper, *it, kinds ) );
    }
    locks.save(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));
    if ( start != Locks::instance().size() )
      zypper.out().info(PL_(
        "Specified lock has been successfully added.",
        "Specified locks have been successfully added.",
        Locks::instance().size() - start));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem adding the package lock:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}


void remove_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds)
{
  try
  {
    Locks & locks = Locks::instance();
    locks.read(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));
    Locks::size_type start = locks.size();
    for_( args_it, args.begin(), args.end() )
    {
      Locks::const_iterator it = locks.begin();
      Locks::LockList::size_type i = 0;
      safe_lexical_cast(*args_it, i);
      if (i > 0 && i <= locks.size())
      {
        advance(it, i-1);
        locks.removeLock(*it);

        zypper.out().info(_("Specified lock has been successfully removed."));
      }
      else //package name
      {
        locks.removeLock( locks::arg2query( zypper, *args_it, kinds ) );
      }
    }

    locks.save(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));

    // nothing removed
    if (start == locks.size())
      zypper.out().info(_("No lock has been removed."));
    //removed something
    else
      zypper.out().info(str::form(PL_(
        "%zu lock has been successfully removed.",
        "%zu locks have been successfully removed.",
        start - locks.size()), start - locks.size()));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem removing the package lock:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}
