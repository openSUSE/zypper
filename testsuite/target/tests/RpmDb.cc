#include "zypp/target/rpm/RpmDb.h"
#include "zypp/Pathname.h"
#include "zypp/Edition.h"
#include "zypp/base/Logger.h"

using namespace zypp::target::rpm;
using namespace zypp;

using std::endl;

typedef std::set<Edition> EditionSet;

int main(int argc, char *argv[])
{
  RpmDb rpm;
  MIL << "Initializing rpm database.." << std::endl;
  rpm.initDatabase(Pathname("/"));
  MIL << "done.." << std::endl;
  
  EditionSet keys = rpm.pubkeyEditions();
  for ( EditionSet::const_iterator it = keys.begin(); it != keys.end(); ++it )
  {
    MIL << "key: " << *it << std::endl;
  }
  return 0;
}
