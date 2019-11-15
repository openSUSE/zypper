#include "TestSetup.h"
#include "zypp/PurgeKernels.h"

#include <boost/test/data/test_case.hpp>

using namespace zypp;
using namespace boost::unit_test;

namespace boost { namespace test_tools { namespace tt_detail {
template<>
struct print_log_value< std::map<std::string, bool> > {
void operator()( std::ostream& ostr,
    std::map<std::string, bool> const& set)
{
  ostr << "{" << std::endl;
  for( const auto &elem : set ) ostr << "'" << elem.first << "'," << std::endl;
  ostr << "}" << std::endl;
}
};
}}}

namespace  {
  std::string makeNVRA( const PoolItem &pck ) {
    return pck.name() + "-" + pck.edition().asString() + "." + pck.arch().asString();
  }

  using TestSample = std::tuple<Pathname, std::string, zypp::Arch, std::string, std::map<std::string, bool> >;

  std::vector<TestSample>  maketestdata() {
    return {
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "oldest,running,latest",
        {
          { "kernel-default-1-2.x86_64", false },
          { "kernel-default-devel-1-2.x86_64", false },
          { "kernel-default-devel-debuginfo-1-2.x86_64", false },
          { "kernel-devel-1-2.noarch", false },
          { "kernel-livepatch-default-1-2.x86_64", false },
          { "kernel-syms-1-2.x86_64", false },
          { "kernel-default-1-4.x86_64", false },
          { "kernel-default-devel-1-4.x86_64", false },
          { "kernel-default-devel-debuginfo-1-4.x86_64", false },
          { "kernel-devel-1-4.noarch", false },
          { "kernel-syms-1-4.x86_64", false },
        }
      },
      //test that keeps only the running kernel
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-1.x86_64", false },
          { "kernel-default-devel-1-1.x86_64", false },
          { "kernel-default-devel-debuginfo-1-1.x86_64", false },
          { "kernel-livepatch-default-1-1.x86_64", false },
          { "kernel-devel-1-1.noarch", false },
          { "kernel-syms-1-1.x86_64", false },
          { "kernel-source-1-1.noarch", false },
          { "kernel-default-1-2.x86_64", false },
          { "kernel-default-devel-1-2.x86_64", false },
          { "kernel-default-devel-debuginfo-1-2.x86_64", false },
          { "kernel-devel-1-2.noarch", false },
          { "kernel-livepatch-default-1-2.x86_64", false },
          { "kernel-syms-1-2.x86_64", false },
          { "kernel-default-1-4.x86_64", false },
          { "kernel-default-devel-1-4.x86_64", false },
          { "kernel-default-devel-debuginfo-1-4.x86_64", false },
          { "kernel-devel-1-4.noarch", false },
          { "kernel-syms-1-4.x86_64", false },
          { "kernel-default-1-5.x86_64", false },
          { "kernel-default-devel-1-5.x86_64", false },
          { "kernel-default-devel-debuginfo-1-5.x86_64", false },
          { "kernel-devel-1-5.noarch", false },
          { "kernel-syms-1-5.x86_64", false },
          { "dummy-kmp-default-1-0.x86_64", false },
        }
      },
      TestSample {
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/simple",
        "1-3-default",
        Arch("x86_64"),
        "oldest+1,running,latest-1",
        {
          { "kernel-default-1-1.x86_64", false },
          { "kernel-livepatch-default-1-1.x86_64", false },
          { "kernel-default-devel-1-1.x86_64", false },
          { "kernel-default-devel-debuginfo-1-1.x86_64", false },
          { "kernel-devel-1-1.noarch", false },
          { "kernel-syms-1-1.x86_64", false },
          { "kernel-source-1-1.noarch", false },
          { "kernel-default-1-5.x86_64", false },
          { "kernel-default-devel-1-5.x86_64", false },
          { "kernel-default-devel-debuginfo-1-5.x86_64", false },
          { "kernel-devel-1-5.noarch", false },
          { "kernel-syms-1-5.x86_64", false },
          { "dummy-kmp-default-1-0.x86_64", false },
        }
      },
      TestSample {
        //kernel-1-1 has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-5-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-2.x86_64", false },
          { "kernel-default-devel-1-2.x86_64", false },
          { "kernel-default-devel-debuginfo-1-2.x86_64", false },
          { "kernel-devel-1-2.noarch", false },
          { "kernel-livepatch-default-1-2.x86_64", false },
          { "kernel-syms-1-2.x86_64", false },
        }
      },
      TestSample {
        //kernel-1-5 provides a symbol for a kmp that has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-1-default",
        Arch("x86_64"),
        "running",
        {
          { "kernel-default-1-2.x86_64", false },
          { "kernel-default-devel-1-2.x86_64", false },
          { "kernel-default-devel-debuginfo-1-2.x86_64", false },
          { "kernel-devel-1-2.noarch", false },
          { "kernel-livepatch-default-1-2.x86_64", false },
          { "kernel-syms-1-2.x86_64", false },
        }
      },
      TestSample {
        //kernel-1-2 is explicitely in the keep spec, it should not be removed
        //kernel-1-5 provides a symbol for a kmp that has a non kernel package depending on it, it should not be removed
        TESTS_SRC_DIR"/zypp/data/PurgeKernels/withdeps",
        "1-1-default",
        Arch("x86_64"),
        "running,1-2",
        {
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
          { "kernel-default-1-1.aarch64", false },
          { "kernel-default-1-1.i686", false },

          /*
          { "kernel-default-devel-1-1.x86_64", false },
          { "kernel-default-devel-debuginfo-1-1.x86_64", false },
          { "kernel-syms-1-1.x86_64", false },
          */

          { "kernel-default-1-2.aarch64", false },
          { "kernel-default-1-2.i686", false },
          { "kernel-default-1-2.x86_64", false },

          { "kernel-default-devel-1-1.aarch64", false },
          { "kernel-default-devel-1-1.i686", false },
          { "kernel-default-devel-1-2.aarch64", false },
          { "kernel-default-devel-1-2.i686", false },
          { "kernel-default-devel-1-2.x86_64", false },

          { "kernel-default-devel-debuginfo-1-1.aarch64", false },
          { "kernel-default-devel-debuginfo-1-1.i686", false },
          { "kernel-default-devel-debuginfo-1-2.aarch64", false },
          { "kernel-default-devel-debuginfo-1-2.i686", false },
          { "kernel-default-devel-debuginfo-1-2.x86_64", false },

          { "kernel-devel-1-2.noarch", false },

          { "kernel-livepatch-default-1-2.aarch64", false },
          { "kernel-livepatch-default-1-2.i686", false },
          { "kernel-livepatch-default-1-2.x86_64", false },

          /*
           Since kernel-syms package requires do not care about architecture, every kernel-syms
           package for flavour-version will be kept as long as a kernel-flavour-devel package of any arch is installed
          { "kernel-syms-1-1.aarch64", false },
          { "kernel-syms-1-1.i686", false },
          */
          { "kernel-syms-1-2.aarch64", false },
          { "kernel-syms-1-2.i686", false },
          { "kernel-syms-1-2.x86_64", false },
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

    auto pck = expectedRemovals.find( makeNVRA(*it) );
    BOOST_REQUIRE_MESSAGE(  pck != expectedRemovals.end(), std::string("Unexpected package removed: ") + makeNVRA(*it) );

    pck->second = true;
  }

  for ( const auto &rem : expectedRemovals ) {
    if (!rem.second)
      std::cout << std::string( "Expected package removal did not happen for: ") + rem.first  << std::endl;
    //BOOST_REQUIRE_MESSAGE( rem.second, std::string( "Expected package removal did not happen for: ") + rem.first );
  }

  BOOST_REQUIRE_EQUAL( expectedRemovals.size(), removeCount );
}
