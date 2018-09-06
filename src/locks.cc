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


template <typename Target, typename Source>
void safe_lexical_cast (Source s, Target &tr) {
  try {
    tr = boost::lexical_cast<Target> (s);
  }
  catch (boost::bad_lexical_cast &) {
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
