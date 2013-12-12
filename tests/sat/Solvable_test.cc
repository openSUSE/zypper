#include <stdio.h>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Easy.h"
#include "zypp/Pattern.h"
#include "zypp/sat/Solvable.h"
#include "TestSetup.h"


#define BOOST_TEST_MODULE Solvable

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


BOOST_AUTO_TEST_CASE(test_init)
{
  TestSetup test( Arch_x86_64 );
  test.loadRepo( TESTS_SRC_DIR "/data/openSUSE-11.1", "opensuse" );
  test.loadRepo( TESTS_SRC_DIR "/data/11.0-update", "update" );
}


BOOST_AUTO_TEST_CASE(attributes)
{
    MIL << sat::Pool::instance();
    Repository r = sat::Pool::instance().reposFind("opensuse");

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
            BOOST_CHECK_EQUAL(p->icon(), "pattern-apparmor");
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

BOOST_AUTO_TEST_CASE(asString)
{
  BOOST_CHECK_EQUAL( sat::Solvable(0).asString(), "noSolvable" );
  BOOST_CHECK_EQUAL( sat::Solvable(1).asString(), "systemSolvable" );
  BOOST_CHECK_EQUAL( sat::Solvable(2).asString(), "product:openSUSE-11.1.x86_64" );
  BOOST_CHECK_EQUAL( sat::Solvable(3693).asString(), "autoyast2-2.16.19-0.1.src" );
  BOOST_CHECK_EQUAL( sat::Solvable(19222).asString(), "noSolvable" );
#if 0
  Repository r = sat::Pool::instance().reposFind("update");
  for_( it, r.solvablesBegin(), r.solvablesEnd() )
  {
    BOOST_CHECK_EQUAL( (*it).asString(), str::numstring((*it).id()) );
  }
#endif
}

BOOST_AUTO_TEST_CASE(SplitIdent)
{
  sat::Solvable::SplitIdent split;
  BOOST_CHECK_EQUAL( split.ident(), IdString() );
  BOOST_CHECK_EQUAL( split.kind(), ResKind() );
  BOOST_CHECK_EQUAL( split.name(), IdString() );

  // - kind defaults to package
  // - package and srcpackage have NO namespaced ident.

  split = sat::Solvable::SplitIdent( 	"foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"foo" );

  split = sat::Solvable::SplitIdent(	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	"package:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::pattern );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	"srcpackage:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );	// !!!
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::srcpackage );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  // now split from kind,name
  // - kind spec in name wins!

  split = sat::Solvable::SplitIdent(	ResKind::package,	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::pattern,	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::pattern );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::srcpackage,	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::srcpackage );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::package,	"package:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::pattern,	"package:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::srcpackage,	"package:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::package );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::package,	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::pattern );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::pattern,	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::pattern );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::srcpackage,	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"pattern:nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::pattern );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::package,	"srcpackage:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::srcpackage );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::pattern,	"srcpackage:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::srcpackage );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

  split = sat::Solvable::SplitIdent(	ResKind::srcpackage,	"srcpackage:nokind:foo" );
  BOOST_CHECK_EQUAL( split.ident(),	"nokind:foo" );
  BOOST_CHECK_EQUAL( split.kind(),	ResKind::srcpackage );
  BOOST_CHECK_EQUAL( split.name(),	"nokind:foo" );

}

BOOST_AUTO_TEST_CASE(duData)
{
  DiskUsageCounter ducounter( DiskUsageCounter::justRootPartition() );

  sat::Solvable s = *sat::WhatProvides( Capability("glibc-devel.x86_64 == 2.8.90-2.3") ).begin();
  BOOST_CHECK_EQUAL( (*ducounter.disk_usage( s ).begin()).pkg_size, 30629 );
}
