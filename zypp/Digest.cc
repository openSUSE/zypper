/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Digest.cc
 *
 * \todo replace by Blocxx
 *
*/


#include <iostream>
#include <sstream>

#ifdef DIGEST_TESTSUITE
#include <fstream>
#endif

#include <zypp/Digest.h>
#include <zypp/base/PtrTypes.h>

using std::endl;

namespace zypp {

    bool DigestReport::askUserToAcceptNoDigest( const zypp::Pathname &file )
    { return false; }

    bool DigestReport::askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    { return false; }

    bool DigestReport::askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    { return false; }

#ifdef DIGEST_TESTSUITE
    int main(int argc, char *argv[])
    {
      using namespace std;
      bool openssl = false;
      unsigned argpos = 1;

      if(argc > 1 && std::string(argv[argpos]) == "--openssl")
      {
        openssl = true;
        ++argpos;
      }

      if(argc - argpos < 2)
      {
        cerr << "Usage: " << argv[0] << " <DIGESTNAME> <FILE>" << endl;
        return 1;
      }

      const char* digestname = argv[argpos++];
      const char* fn = argv[argpos++];

      std::ifstream file(fn);

      std::string digest = Digest::digest(digestname, file);

      if(openssl)
        cout << digestname << "(" << fn << ")= " << digest << endl;
      else
        cout << digest << "  " << fn << endl;

      return 0;
    }
#endif

} // namespace zypp
