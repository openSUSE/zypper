#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include <satsolver/solvable.h>
#include "zypp/base/Logger.h"
#include "zypp/base/Easy.h"
#include "zypp/ZYppFactory.h"
#include "zypp/Pattern.h"
#include "zypp/sat/Solvable.h"

#define BOOST_TEST_MODULE Solvable

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


static void init_pool()
{
  Pathname dir(TESTS_SRC_DIR);
  dir += "/zypp/data/PoolQuery";

  ZYpp::Ptr z = getZYpp();
  ZConfig::instance().setSystemArchitecture(Arch("i586"));

  RepoInfo i1; i1.setAlias("factory");
  sat::Pool::instance().addRepoSolv(dir / "factory.solv", i1);
}

BOOST_AUTO_TEST_CASE(attributes)
{
    init_pool();
    MIL << sat::Pool::instance();
    Repository r = sat::Pool::instance().reposFind("factory");
    
    int c = 0;
    
    for ( Repository::SolvableIterator it = r.solvablesBegin();
          it != r.solvablesEnd();
          ++it )
    {
        sat::Solvable s = *it;    
        //MIL << s.ident() << endl;
        if ( s.ident() == "pattern:apparmor" )
        {             
            c++;
            
            //  solvable 21795 (21796):
            // name: pattern:apparmor 11.0-67 i586
            // vendor: SUSE LINUX Products GmbH, Nuernberg, Germany
            // provides:
            //   pattern:apparmor = 11.0-67
            // requires:
            //   pattern:basesystem
            //   apparmor-parser
            //   audit
            //   apparmor-profiles
            // recommends:
            //   yast2-apparmor
            //   apparmor-utils
            //   pattern:apparmor_opt
            // solvable:category: Base Technologies
            // solvable:icon: yast-software
            // solvable:summary: Novell AppArmor
            // solvable:description: Novell AppArmor is an application security framework that provides mandatory access control for programs. It protects from exploitation of software flaws and compromised systems. It offers an advanced tool set that automates the development of per-program application security without requiring additional knowledge.
            // solvable:isvisible: 1
            // solvable:order: 1030
            Pattern::Ptr p = asKind<Pattern>(makeResObject(s));
            BOOST_CHECK(p);
            BOOST_CHECK_EQUAL(p->name(), "apparmor");
            BOOST_CHECK_EQUAL(p->vendor(), "SUSE LINUX Products GmbH, Nuernberg, Germany");
            BOOST_CHECK_EQUAL(p->category(), "Base Technologies");
            BOOST_CHECK_EQUAL(p->summary(), "Novell AppArmor");
            BOOST_CHECK_EQUAL(p->icon(), "yast-software");
            BOOST_CHECK_EQUAL(p->userVisible(), true);
            BOOST_CHECK_EQUAL(p->isDefault(), false);
        }
        if ( s.ident() == "pattern:default" )
        {             
            c++;
            Pattern::Ptr p = asKind<Pattern>(makeResObject(s));
            BOOST_CHECK(p);
            BOOST_CHECK_EQUAL(p->userVisible(), false);
        }
    }
    
    // check that we actually found all testeable
    // resolvables
    BOOST_CHECK_EQUAL(c, 2);
    
            
}
