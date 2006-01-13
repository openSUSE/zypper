#include "zypp/target/hal/Hal.h"

using namespace zypp::target::hal;

int
main (int argc, char *argv[])
{
  Hal hal = Hal::instance();

  if (!hal.query ("processor"))
	return 1;

  if (!hal.query ("processor", zypp::Rel::GT, "0"))		// not supported yet
	return 2;

  return 0;
}
