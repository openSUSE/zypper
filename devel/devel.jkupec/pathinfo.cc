#include <iostream>

#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;

int main (int argc, const char ** argv)
{
  Pathname datadir(SRC_DIR "/data/pathinfo");
  Pathname alink = datadir / "alink";
  PathInfo alinkinfo(alink);

  cout << "alink exists: " << alinkinfo.isExist() << endl;
  Pathname alinkExp = filesystem::expandlink(alink);
  cout << "alink expands to: " << alinkExp << endl;
  cout << "alinkExp exists: " << PathInfo(alinkExp).isExist() << endl;

  Pathname subdirlink = datadir / "subdirlink";
  PathInfo subdirlinkinfo(subdirlink);

  cout << "subdirlink exists: " << subdirlinkinfo.isExist() << endl;
  Pathname subdirlinkExp = filesystem::expandlink(subdirlink);
  cout << "subdirlink expands to: " << subdirlinkExp << endl;
  cout << "subdirlinkExp exists: " << PathInfo(subdirlinkExp).isExist() << endl;
}
