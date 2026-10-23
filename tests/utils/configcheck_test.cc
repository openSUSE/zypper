#include <tests/lib/TestSetup.h>
#include "commands/utils/configcheck.h"
#include <zypp-core/fs/PathInfo.h>

using namespace zypp;

BOOST_AUTO_TEST_CASE(configcheck_test)
{
  ConfigCheck checker;
  std::vector<std::string> dummyPaths = { "/tmp/config1", "/tmp/config2", "/tmp/config3" };
  
  zypp::filesystem::assert_file("/tmp/config1.rpmnew");
  zypp::filesystem::assert_file("/tmp/config2.rpmsave");

  // config3 has no .rpmnew or .rpmsave

  // We invoke the command logic using the static instance.
  Zypper &zypper = Zypper::instance();
  bool success = checker.run(zypper, dummyPaths);
  BOOST_CHECK_EQUAL(success, true);
  
  zypp::filesystem::unlink("/tmp/config1.rpmnew");
  zypp::filesystem::unlink("/tmp/config2.rpmsave");
}
