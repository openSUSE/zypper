#include <iostream>
#include <boost/lexical_cast.hpp>

#include "zypp/base/String.h"
#include "zypp/Locks.h"

#include "output/Out.h"
#include "zypper-main.h" 
#include "zypper-tabulator.h"
#include "zypper-utils.h"
#include "zypper-locks.h"

using namespace zypp;
using namespace std;

static const string
get_string_for_table(const set<string> & repos)
{
  if (repos.empty())
    return _("(any)");
  else if (repos.size() > 1)
    return _("(multiple)");
  else
    return *repos.begin();
}

void list_locks(Zypper & zypper)
{
  try
  {
    Locks & locks = Locks::instance();
    locks.read();

    Table t;

    TableHeader th;
    th << "#" << _("Name");
    if (zypper.globalOpts().is_rug_compatible)
      th << _("Catalog") << _("Importance");
    else
      th << _("Type") << _("Repository");
    
    t << th;

    unsigned i = 1;
    for_(it, locks.begin(), locks.end())
    {
      TableRow tr;
      
      // #
      tr << str::numstring (i);

      // name
      PoolQuery::StrContainer attr = it->attribute(sat::SolvAttr::name);
      it->strings();
      if (attr.size() + it->strings().size() > 1)
        // translators: locks table value
        tr << _("(multiple)");
      else if (attr.empty() && it->strings().empty())
        // translators: locks table value
        tr << _("(any)");
      else if (attr.empty())
        tr << *it->strings().begin();
      else
        tr << *attr.begin();

      if (it->repos().empty())
      {
        
      }

      set<string> strings;
      if (zypper.globalOpts().is_rug_compatible)
      {
        // catalog
        copy(it->repos().begin(), it->repos().end(), inserter(strings, strings.end()));
        tr << get_string_for_table(strings);
        // importance
        tr << _("(any)");
      }
      else
      {
        // type
        for_(kit, it->kinds().begin(), it->kinds().end())
          strings.insert(kit->asString());
        tr << get_string_for_table(strings);
        // repo
        strings.clear();
        copy(it->repos().begin(), it->repos().end(), inserter(strings, strings.end()));
        tr << get_string_for_table(strings);
      }

      t << tr;
      ++i;
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
    PoolQuery q;
    q.addAttribute(sat::SolvAttr::name, args[0]);
    for_(it, kinds.begin(), kinds.end())
      q.addKind(*it);
    q.setMatchGlob();
    //! \todo addRepo()
    q.setCaseSensitive();

    Locks & locks = Locks::instance();
    locks.readAndApply();
    locks.addLock(q);
    locks.save();
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem adding the package lock:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  zypper.out().info(_("Specified lock has been successfully added."));
}


void remove_locks(Zypper & zypper, const Zypper::ArgList & args)
{
  try
  {
    Locks & locks = Locks::instance();
    locks.readAndApply();
    Locks::const_iterator it = locks.begin();
    Locks::LockList::size_type i = 0;
    safe_lexical_cast(args[0], i);
    if (i > 0 && i <= locks.size())
    {
      advance(it, i-1);
      locks.removeLock(*it);
      locks.save();

      zypper.out().info(_("Specified lock has been successfully removed."));
    }
    else
      zypper.out().error(str::form(_("Invalid lock number: %s"), args[0].c_str()));
  }
  catch(const Exception & e)
  {
    ZYPP_CAUGHT(e);
    zypper.out().error(e, _("Problem adding the package lock:"));
    zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }
}
