#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/base/Logger.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/TranslatedText.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/target/rpm/RpmDb.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>
#include <zypp/KeyRing.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;



//using namespace DbXml;

int main()
{
  //ZYpp::Ptr z = getZYpp();
  getZYpp()->keyRing()->importKey("duncan.asc", true);
  zypp::target::rpm::RpmDb rpm;
  rpm.initDatabase();
  
  std::set<Edition> rpm_keyse = rpm.pubkeyEditions();
  for (std::set<Edition>::const_iterator it = rpm_keyse.begin(); it != rpm_keyse.end(); it++)
  {
    MIL << "RPM key edition: " << (*it).version() << std::endl;
  }
  MIL << "-----------------------------------------------------------" << std::endl;
  std::list<PublicKey> rpm_keys = rpm.pubkeys();
  for (std::list<PublicKey>::const_iterator it = rpm_keys.begin(); it != rpm_keys.end(); it++)
  {
    MIL << "RPM key: [" << (*it).id << "] [" << (*it).name << "] [" << (*it).fingerprint << "]" << std::endl;
  }
  MIL << "-----------------------------------------------------------" << std::endl;
  std::list<PublicKey> keys = getZYpp()->keyRing()->trustedPublicKeys();
  for (std::list<PublicKey>::const_iterator it = keys.begin(); it != keys.end(); it++)
  {
    MIL << "Trusted key: [" << (*it).id << "] [" << (*it).name << "] [" << (*it).fingerprint << "]" << std::endl;
  } 
  
//   KeyRing kr(Pathname("./keyring/all"), Pathname("./keyring/trusted"));
//   kr.importKey(Pathname("content.key"));
//   kr.verifyFileSignatureWorkflow(Pathname("repomd.xml.asc"), Pathname("repomd.xml.asc")); 
//   exit(0);
//   
//   KeyRing kr(Pathname("./keyring/all"), Pathname("./keyring/trusted"));
//   kr.importKey(Pathname("content.key"));
//   kr.verifyFileSignatureWorkflow(Pathname("content.asc"), Pathname("content.asc")); 
//   exit(0);
//  
  
//   std::string sid = kr.readSignatureKeyId("content", "content.asc");
//   MIL << "Signature: " << sid << std::endl;
//   
//   PublicKey key = kr.readPublicKey("./content.key");
//   MIL << "Read: " << key.id << " " << key.name << std::endl;
//   
//   kr.importKey(Pathname("content.key"));
//   std::list<PublicKey> keys;
//   keys = kr.publicKeys();
//   
//   bool verifies;
//   verifies = kr.verifyFileSignature( Pathname("./content"), Pathname("./content.asc"));
//   MIL << "verifying " << verifies << std::endl;
//   
//   for (std::list<PublicKey>::const_iterator it = keys.begin(); it != keys.end(); it++)
//   {
//     MIL << (*it).id << " " << (*it).name << std::endl;
//     MIL << "now delete it" << std::endl;
//     kr.deleteKey((*it).id);
//   } 
//   
//   verifies = kr.verifyFileSignature( Pathname("./content"), Pathname("./content.asc"));
//   MIL << "verifying " << verifies << std::endl;
//   
//   MIL << "now delete a non.existant key" << std::endl;
//   kr.deleteKey("ADhjghjghg");
//   
  
  
}


