#include "TestSetup.h"
#include <zypp/PurgeKernels.h>
#include <set>

#include <boost/test/data/test_case.hpp>

using namespace zypp;
using namespace boost::unit_test;

namespace  {
  // Defines a expected removal of a package and carries the flag if it actually was removed
  struct ExpectedRemoval {
    ExpectedRemoval( std::string &&name ) : packageNVRA(std::move(name)) {}
    std::string packageNVRA;
    mutable bool wasRemoved = false;
  };

  bool operator< ( const ExpectedRemoval &a, const ExpectedRemoval &b ) {
    return a.packageNVRA < b.packageNVRA;
  }
}

namespace boost { namespace test_tools { namespace tt_detail {
template<>
struct print_log_value< std::set<ExpectedRemoval> > {
void operator()( std::ostream& ostr,
    std::set<ExpectedRemoval> const& set)
{
  ostr << "{" << std::endl;
  for( const auto &elem : set ) ostr << "'" << elem.packageNVRA << "'," << std::endl;
  ostr << "}" << std::endl;
}
};
}}}

namespace  {
  std::string makeNVRA( const PoolItem &pck ) {
    return pck.name() + "-" + pck.edition().asString() + "." + pck.arch().asString();
  }

  using TestSample = std::tuple<
    Pathname,    // repoPath
    std::string, // uname_r
    zypp::Arch,  // arch
    std::string, // keepSpec
    std::set<ExpectedRemoval> // expectedRems
    >;

  std::vector<TestSample>  maketestdata() {
    return {
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "oldest,running,latest",
        {
          { "kernel-default-1-2.x86_64" },
          { "kernel-default-devel-1-2.x86_64" },
          { "kernel-default-devel-debuginfo-1-2.x86_64" },
          { "kernel-devel-1-2.noarch" },
          { "kernel-livepatch-default-1-2.x86_64" },
          { "kernel-syms-1-2.x86_64" },
          { "kernel-default-1-4.x86_64" },
          { "kernel-default-devel-1-4.x86_64" },
          { "kernel-default-devel-debuginfo-1-4.x86_64" },
          { "kernel-devel-1-4.noarch" },
          { "kernel-syms-1-4.x86_64" },
          // left over devel packages that need to go away too
          { "kernel-devel-1-1.2.noarch" },
          { "kernel-source-1-1.2.noarch" },
          { "kernel-default-devel-1-3.x86_64" },
          { "kernel-default-devel-debuginfo-1-3.x86_64" },
          { "kernel-devel-1-3.noarch" },
        }
      },
      //test that keeps only the running kernel
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-1.x86_64" },
          { "kernel-default-devel-1-1.x86_64" },
          { "kernel-default-devel-debuginfo-1-1.x86_64" },
          { "kernel-livepatch-default-1-1.x86_64" },
          { "kernel-devel-1-1.noarch" },
          { "kernel-syms-1-1.x86_64" },
          { "kernel-source-1-1.noarch" },
          { "kernel-default-1-2.x86_64" },
          { "kernel-default-devel-1-2.x86_64" },
          { "kernel-default-devel-debuginfo-1-2.x86_64" },
          { "kernel-devel-1-2.noarch" },
          { "kernel-livepatch-default-1-2.x86_64" },
          { "kernel-syms-1-2.x86_64" },
          { "kernel-default-1-4.x86_64" },
          { "kernel-default-devel-1-4.x86_64" },
          { "kernel-default-devel-debuginfo-1-4.x86_64" },
          { "kernel-devel-1-4.noarch" },
          { "kernel-syms-1-4.x86_64" },
          { "kernel-default-1-5.x86_64" },
          { "kernel-default-devel-1-5.x86_64" },
          { "kernel-default-devel-debuginfo-1-5.x86_64" },
          { "kernel-devel-1-5.noarch" },
          { "kernel-syms-1-5.x86_64" },
          { "dummy-kmp-default-1-0.x86_64" },
          // left over devel packages that need to go away too
          { "kernel-devel-1-1.2.noarch" },
          { "kernel-source-1-1.2.noarch" },
          { "kernel-default-devel-1-3.x86_64" },
          { "kernel-devel-1-3.noarch" },
          { "kernel-default-devel-1-3.x86_64" },
          { "kernel-default-devel-debuginfo-1-3.x86_64" },
        }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "oldest+1,running,latest-1",
        {
          { "kernel-default-1-1.x86_64" },
          { "kernel-livepatch-default-1-1.x86_64" },
          { "kernel-default-devel-1-1.x86_64" },
          { "kernel-default-devel-debuginfo-1-1.x86_64" },
          { "kernel-devel-1-1.noarch" },
          { "kernel-syms-1-1.x86_64" },
          { "kernel-source-1-1.noarch" },
          { "kernel-default-1-5.x86_64" },
          { "kernel-default-devel-1-5.x86_64" },
          { "kernel-default-devel-debuginfo-1-5.x86_64" },
          { "kernel-devel-1-5.noarch" },
          { "kernel-syms-1-5.x86_64" },
          { "dummy-kmp-default-1-0.x86_64" },
          // left over devel packages that need to go away too
          { "kernel-default-devel-1-3.x86_64" },
          { "kernel-default-devel-debuginfo-1-3.x86_64" },
          { "kernel-devel-1-3.noarch" },
        }
      },
      TestSample {
        //kernel-1-1 has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-5-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-2.x86_64" },
          { "kernel-default-extra-1-2.x86_64" },
          { "kernel-default-devel-1-2.x86_64" },
          { "kernel-default-devel-debuginfo-1-2.x86_64" },
          { "kernel-devel-1-2.noarch" },
          { "kernel-livepatch-default-1-2.x86_64" },
          { "kernel-syms-1-2.x86_64" },
          // the following packages are not held back because they do not fit keep spec and no deps are keeping them
          { "kernel-default-devel-1-1.x86_64" },
          { "kernel-default-devel-debuginfo-1-1.x86_64" },
          { "kernel-devel-1-1.noarch" },
          { "kernel-syms-1-1.x86_64" },
        }
      },
      TestSample {
        //kernel-1-5 provides a symbol for a kmp that has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-1-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-2.x86_64" },
          { "kernel-default-extra-1-2.x86_64" },
          { "kernel-default-devel-1-2.x86_64" },
          { "kernel-default-devel-debuginfo-1-2.x86_64" },
          { "kernel-devel-1-2.noarch" },
          { "kernel-livepatch-default-1-2.x86_64" },
          { "kernel-syms-1-2.x86_64" },
          { "kernel-default-devel-1-5.x86_64" },
          { "kernel-default-devel-debuginfo-1-5.x86_64" },
          { "kernel-devel-1-5.noarch" },
          { "kernel-syms-1-5.x86_64" },
        }
      },
      TestSample {
        //kernel-1-2 is explicitly in the keep spec, it should not be removed
        //kernel-1-5 provides a symbol for a kmp that has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-1-default",
        Arch("x86_64"),
        "running,1-2",
        {
          { "kernel-default-devel-1-5.x86_64" },
          { "kernel-default-devel-debuginfo-1-5.x86_64" },
          { "kernel-devel-1-5.noarch" },
          { "kernel-syms-1-5.x86_64" },
        }
      },
      TestSample {
        //kernel-default-1-1.x86_64 is the running kernel it should not be removed,
        //in all sets with different arch only the latest should be kept
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/arch",
        "1-1-default",
        Arch("x86_64"),
        "latest,running",
        {
          { "kernel-default-1-1.aarch64" },
          { "kernel-default-1-1.i686" },

          //{ "kernel-syms-1-1.x86_64" },
          //{ "kernel-default-devel-1-1.x86_64" },
          //{ "kernel-default-devel-debuginfo-1-1.x86_64" },

          { "kernel-default-1-2.aarch64" },
          { "kernel-default-1-2.i686" },
          { "kernel-default-1-2.x86_64" },

          { "kernel-default-devel-1-1.aarch64" },
          { "kernel-default-devel-1-1.i686" },
          { "kernel-default-devel-1-2.aarch64" },
          { "kernel-default-devel-1-2.i686" },
          { "kernel-default-devel-1-2.x86_64" },

          { "kernel-default-devel-debuginfo-1-1.aarch64" },
          { "kernel-default-devel-debuginfo-1-1.i686" },
          { "kernel-default-devel-debuginfo-1-2.aarch64" },
          { "kernel-default-devel-debuginfo-1-2.i686" },
          { "kernel-default-devel-debuginfo-1-2.x86_64" },

          { "kernel-devel-1-2.noarch" },

          { "kernel-livepatch-default-1-2.aarch64" },
          { "kernel-livepatch-default-1-2.i686" },
          { "kernel-livepatch-default-1-2.x86_64" },

          { "kernel-syms-1-1.aarch64" },
          { "kernel-syms-1-1.i686" },

          { "kernel-syms-1-2.aarch64" },
          { "kernel-syms-1-2.i686" },
          { "kernel-syms-1-2.x86_64" },
        }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/rebuild",
        "1-1-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-source-1-1.noarch" },
        }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/fancybuildnr",
        "5.8.1-3.g846658e-default",
        Arch("x86_64"),
        "latest,latest-1,running",
        {
          { "kernel-default-5.7.8-1.1.g8f507a0.x86_64" },
          { "kernel-default-5.7.9-1.1.ga010166.x86_64" },
          { "kernel-default-5.7.10-1.1.g6a1b5cf.x86_64" },
          { "kernel-default-5.7.10-3.1.gd1148b9.x86_64" },
          { "kernel-default-5.7.11-1.1.g5015994.x86_64" },
          { "kernel-default-5.7.12-1.1.g9c98feb.x86_64" },
          { "kernel-default-5.8.0-1.1.gd3bf2d6.x86_64" },
          { "kernel-default-5.8.0-2.1.g9bc0044.x86_64" },
          { "kernel-default-5.8.0-3.1.gd4e7682.x86_64" },
          { "kernel-default-5.8.1-1.1.ge6658c9.x86_64" },
          // those are running, latest and latest-1 , they should stay
          //{ "kernel-default-5.8.1-2.1.g553537d.x86_64" },
          //{ "kernel-default-5.8.1-3.1.g846658e.x86_64" },
          }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/kernel-base",
        "5.7.8-3-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-base-5.7.8-1.1.1.1.x86_64" },
          { "kernel-default-base-5.7.8-2.1.1.1.x86_64" },
          //{ "kernel-default-base-5.7.8-3.1.1.1.x86_64" },
          { "kernel-default-base-5.8.8-2.1.1.1.x86_64" },
          }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/kernel-base",
        "5.7.8-3-default",
        Arch("x86_64"),
        "running, 5.7.8-2.1.1",
        {
          { "kernel-default-base-5.7.8-1.1.1.1.x86_64" },
          { "kernel-default-base-5.8.8-2.1.1.1.x86_64" },
          //{ "kernel-default-base-5.7.8-2.1.1.1.x86_64" },
          //{ "kernel-default-base-5.7.8-3.1.1.1.x86_64" },
          }
      },
      // same test set as before, but this time the user took the uname-r kernel output
      // we should ignore the -flavor in the end and just use the edition part
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/kernel-base",
        "5.7.8-3-default",
        Arch("x86_64"),
        "running, 5.7.8-2.1.1-default",
        {
          { "kernel-default-base-5.7.8-1.1.1.1.x86_64" },
          { "kernel-default-base-5.8.8-2.1.1.1.x86_64" },
          //{ "kernel-default-base-5.7.8-2.1.1.1.x86_64" },
          //{ "kernel-default-base-5.7.8-3.1.1.1.x86_64" },
          }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/flavour",
        "1-3-rt",
        Arch("x86_64"),
        "running",
        {
          { "kernel-rt-1-1.x86_64" },
          { "kernel-rt-devel-1-1.x86_64" },
          { "kernel-rt-devel-debuginfo-1-1.x86_64" },
          { "kernel-devel-rt-1-1.noarch" },
          { "kernel-syms-rt-1-1.x86_64" },
          { "kernel-source-rt-1-1.noarch" },
          { "kernel-rt-1-2.x86_64" },
          { "kernel-rt-devel-1-2.x86_64" },
          { "kernel-rt-devel-debuginfo-1-2.x86_64" },
          { "kernel-devel-rt-1-2.noarch" },
          { "kernel-syms-rt-1-2.x86_64" },
          { "kernel-rt-1-4.x86_64" },
          { "kernel-rt-devel-1-4.x86_64" },
          { "kernel-rt-devel-debuginfo-1-4.x86_64" },
          { "kernel-devel-rt-1-4.noarch" },
          { "kernel-syms-rt-1-4.x86_64" },
          { "kernel-rt-1-5.x86_64" },
          { "kernel-rt-devel-1-5.x86_64" },
          { "kernel-rt-devel-debuginfo-1-5.x86_64" },
          { "kernel-devel-rt-1-5.noarch" },
          { "kernel-syms-rt-1-5.x86_64" },
          // left over devel packages that need to go away too
          { "kernel-devel-rt-1-1.2.noarch" },
          { "kernel-source-rt-1-1.2.noarch" },
          { "kernel-rt-devel-1-3.x86_64" },
          { "kernel-devel-rt-1-3.noarch" },
          { "kernel-rt-devel-1-3.x86_64" },
          { "kernel-rt-devel-debuginfo-1-3.x86_64" },
        }
      },
      // rc kernels have a special naming hack because upstream uses characters that are forbidden in rpm versions
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/rckrnl",
        "5.13.0-rc7-17-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-5.13.rc7-16.1.x86_64" },
          { "kernel-default-5.13~rc7-18.1.x86_64" }
        }
      },
    };
  }
}

namespace bdata = boost::unit_test::data;

BOOST_DATA_TEST_CASE(purge_kernels, bdata::make( maketestdata() ), repoPath, uname_r, arch, keepSpec, expectedRems )
{
  TestSetup test( Arch_x86_64 );
  test.loadTestcaseRepos( repoPath );

  auto expectedRemovals = expectedRems;

  PurgeKernels krnls;
  krnls.setUnameR( uname_r );
  krnls.setKernelArch( arch );
  krnls.setKeepSpec( keepSpec );
  krnls.markObsoleteKernels();

  auto pool = ResPool::instance();
  BOOST_REQUIRE( pool.resolver().resolvePool() );

  unsigned removeCount = 0;
  const filter::ByStatus toBeUninstalledFilter( &ResStatus::isToBeUninstalled );
  for ( auto it = pool.byStatusBegin( toBeUninstalledFilter ); it != pool.byStatusEnd( toBeUninstalledFilter );  it++  ) {
    removeCount++;

    std::set<ExpectedRemoval>::iterator pck = expectedRemovals.find( makeNVRA(*it) );
    BOOST_REQUIRE_MESSAGE(  pck != expectedRemovals.end(), std::string("Unexpected package removed: ") + makeNVRA(*it) + (it->status().isByUser() ? " (by user)" : " (autoremoved)") );

    (*pck).wasRemoved = true;
  }

  for ( const auto &rem : expectedRemovals ) {
    if (!rem.wasRemoved)
      std::cout << std::string( "Expected package removal did not happen for: ") + rem.packageNVRA  << std::endl;
    //BOOST_REQUIRE_MESSAGE( rem.second, std::string( "Expected package removal did not happen for: ") + rem.first );
  }

  BOOST_REQUIRE_EQUAL( expectedRemovals.size(), removeCount );
}
