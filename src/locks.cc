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

///////////////////////////////////////////////////////////////////
namespace
{
  inline std::string get_string_for_table( const std::set<std::string> & attrvals )
  {
    std::string ret;
    if ( attrvals.empty() )
      ret = _("(any)");
    else if ( attrvals.size() > 1 )
      ret = _("(multiple)");
    else
      ret = *attrvals.begin();
    return ret;
  }

  inline int compareByN( const sat::Solvable & lhs, const sat::Solvable & rhs )	// from zypp::sat::
  {
    int res = 0;
    if ( lhs != rhs )
    {
      if ( (res = lhs.kind().compare( rhs.kind() )) == 0 )
	res = lhs.name().compare( rhs.name() );
    }
    return res;
  }

  inline std::string getLockDetails( const PoolQuery & q )
  {
    if ( q.empty() )
      return "";

    PropertyTable p;
    {
      struct DoCompare
      {
	int operator()( const sat::Solvable & lhs, const sat::Solvable & rhs ) const
	{ return( doComapre( lhs, rhs ) < 0 ); }

	int doComapre( const sat::Solvable & lhs, const sat::Solvable & rhs ) const
	{
	  // return zypp::sat::compareByNVRA( lhs, rhs );
	  // do N(<) A(>) VR(>)
	  int res = compareByN( lhs, rhs );			// ascending  l<r
	  if ( res == 0 )
	    res = rhs.arch().compare( lhs.arch() );		// descending r<l
	  if ( res == 0 )
	    res = rhs.edition().compare( lhs.edition() );	// descending r<l
	  if ( res == 0 )
	    res = lhs.repository().asUserString().compare( rhs.repository().asUserString() );	// ascending  l<r
	  return res;
	}
      };
      std::set<sat::Solvable,DoCompare> i;
      std::set<sat::Solvable,DoCompare> a;
      for ( const auto & solv : q )
      { (solv.isSystem()?i:a).insert( solv ); }

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
    return str::Str() << p;
  }

} //namespace
///////////////////////////////////////////////////////////////////

void list_locks(Zypper & zypper)
{
  shared_ptr<ListLocksOptions> listLocksOptions = zypper.commandOptionsOrDefaultAs<ListLocksOptions>();

  bool withSolvables = listLocksOptions->_withSolvables;
  bool withMatches = withSolvables||listLocksOptions->_withMatches;

  try
  {
    Locks & locks = Locks::instance();
    locks.read( Pathname::assertprefix( zypper.globalOpts().root_dir, ZConfig::instance().locksFile() ) );

    Table t;

    TableHeader th;
    th << "#" << _("Name");
    if ( withMatches )
      th << _("Matches");
    if (zypper.globalOpts().is_rug_compatible)
      th << _("Repository") << _("Importance");
    else
      th << _("Type") << _("Repository");

    t << th;

    unsigned i = 0;
    for ( const PoolQuery & q : locks )
    {
      TableRow tr;
      ++i;

      // #
      tr << str::numstring( i );

      // name
      const PoolQuery::StrContainer & nameStings( q.attribute( sat::SolvAttr::name ) );
      const PoolQuery::StrContainer & globalStrings( q.strings() );

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
      if ( withMatches )
	tr << q.size();

      std::set<std::string> strings;
      if (zypper.globalOpts().is_rug_compatible)
      {
        // repository
        copy(q.repos().begin(), q.repos().end(), inserter(strings, strings.end()));
        tr << get_string_for_table(strings);
        // importance
        tr << _("(any)");
      }
      else
      {
	// type
	for ( const ResKind & kind : q.kinds() )
	  strings.insert( kind.asString() );
	tr << get_string_for_table( strings );

	// repo
	strings.clear();
	copy( q.repos().begin(), q.repos().end(), inserter(strings, strings.end()) );
	tr << get_string_for_table( strings );
      }
      
      // opt Solvables
      if ( withSolvables )
      {
	tr.addDetail( getLockDetails( q ) );
      }

      t << tr;
    }

    if (t.empty())
      zypper.out().info(_("There are no package locks defined."));
    else
      cout << t;
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Error reading the locks file:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
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
      PoolQuery q;
      if ( kinds.empty() ) // derive it from the name
      {
        sat::Solvable::SplitIdent split( *it );
        q.addAttribute( sat::SolvAttr::name, split.name().asString() );
        q.addKind( split.kind() );
      }
      else
      {
        q.addAttribute(sat::SolvAttr::name, *it);
        for_(itk, kinds.begin(), kinds.end()) {
          q.addKind(*itk);
        }
      }
      q.setMatchGlob();
      parsed_opts::const_iterator itr;
      //TODO rug compatibility for more arguments with version restrict
      if ((itr = copts.find("repo")) != copts.end())
      {
        for_(it_repo,itr->second.begin(), itr->second.end())
        {
          RepoInfo info;
          if( match_repo( zypper, *it_repo, &info))
            q.addRepo(info.alias());
          else //TODO some error handling
            WAR << "unknown repository" << *it_repo << endl;
        }
      }
      q.setCaseSensitive();

      locks.addLock(q);
    }
    locks.save(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));
    if ( start != Locks::instance().size() )
      zypper.out().info(_PL(
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
        //TODO fill query in one method to have consistent add/remove
        //TODO what to do with repo and kinds?
        PoolQuery q;
	if ( kinds.empty() ) // derive it from the name
	{
	  // derive kind from the name: (rl should also support -t)
	  sat::Solvable::SplitIdent split( *args_it );
	  q.addAttribute( sat::SolvAttr::name, split.name().asString() );
	  q.addKind( split.kind() );
	}
	else
	{
	  q.addAttribute(sat::SolvAttr::name, *args_it);
	  for_(itk, kinds.begin(), kinds.end()) {
	    q.addKind(*itk);
	  }
	}
	q.setMatchGlob();
        parsed_opts::const_iterator itr;
        if ((itr = copts.find("repo")) != copts.end())
        {
          for_(it_repo,itr->second.begin(), itr->second.end())
          {
            RepoInfo info;
            if( match_repo( zypper, *it_repo, &info))
              q.addRepo(info.alias());
            else //TODO some error handling
              WAR << "unknown repository" << *it_repo << endl;
          }
        }
        q.setCaseSensitive();

        locks.removeLock(q);
      }
    }

    locks.save(Pathname::assertprefix
        (zypper.globalOpts().root_dir, ZConfig::instance().locksFile()));

    // nothing removed
    if (start == locks.size())
      zypper.out().info(_("No lock has been removed."));
    //removed something
    else
      zypper.out().info(str::form(_PL(
        "%zu lock has been successfully removed.",
        "%zu locks have been succesfully removed.",
        start - locks.size()), start - locks.size()));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem removing the package lock:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}
