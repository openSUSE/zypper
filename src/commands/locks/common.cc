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

///////////////////////////////////////////////////////////////////
namespace locks
{
  zypp::PoolQuery arg2query( Zypper & zypper, const std::string & arg_r, const std::set<zypp::ResKind> & kinds_r, const std::vector<std::string> & repos_r )
  {
    PoolQuery q;
    if ( kinds_r.empty() ) // derive it from the name
    {
      sat::Solvable::SplitIdent split( arg_r );
      q.addAttribute( sat::SolvAttr::name, split.name().asString() );
      q.addKind( split.kind() );
    }
    else
    {
      q.addAttribute(sat::SolvAttr::name, arg_r);
      for_(itk, kinds_r.begin(), kinds_r.end()) {
	q.addKind(*itk);
      }
    }
    q.setMatchGlob();
    parsed_opts::const_iterator itr;
    for_(it_repo, repos_r.begin(), repos_r.end())
    {
      RepoInfo info;
      if( match_repo( zypper, *it_repo, &info))
	q.addRepo(info.alias());
      else //TODO some error handling
	WAR << "unknown repository" << *it_repo << endl;
    }
    q.setCaseSensitive();
    return q;
  }

} // namespace locks
///////////////////////////////////////////////////////////////////
