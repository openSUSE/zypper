/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
/** \file commands/locks/common.cc
 * Common code used by different commands.
 */
#include "commands/locks/common.h"
#include "repos.h"

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

  PoolQuery arg2query( Zypper & zypper, const std::string & arg_r, const std::set<ResKind> & kinds_r, const std::vector<std::string> & repos_r, const std::string & comment_r )
  {
    // Try to stay with the syntax the serialized query (AKA lock) generates.
    //     type: package
    //     match_type: glob
    //     case_sensitive: on
    //     solvable_name: kernel

    PoolQuery q;
    q.setMatchGlob();
    q.setCaseSensitive();

    for_( it, repos_r.begin(), repos_r.end() )
    {
      RepoInfo info;
      if ( match_repo( zypper, *it, &info ) )
        q.addRepo( info.alias() );
      else //TODO some error handling
        WAR << "unknown repository" << *it << endl;
    }
    q.setComment(comment_r);

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
