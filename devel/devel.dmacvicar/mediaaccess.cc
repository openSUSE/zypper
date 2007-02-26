#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/media/MediaAccess.h>
#include <zypp/media/MediaManager.h>
#include <zypp/MediaSetAccess.h>

#include "zypp/Product.h"
#include "zypp/Package.h"


using namespace std;
using namespace zypp;
using namespace media;

class SimpleVerifier : public zypp::media::MediaVerifierBase
{
  public:
  
  SimpleVerifier( const std::string &id )
  {
    _media_id = id;
  }

  virtual bool isDesiredMedia(const media::MediaAccessRef &ref)
  {
    return ref->doesFileExist(Pathname("/." + _media_id ));
  }
  
  private:
    std::string _media_id;
    media::MediaNr _media_nr;
};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      MediaSetAccess access(Url("dir:/home/duncanmv/tmp/url/CD1"), "/");
      access.setVerifier( 1, media::MediaVerifierRef(
                            new SimpleVerifier("cd1") )
                        );
      access.setVerifier( 2, media::MediaVerifierRef(
                            new SimpleVerifier("cd2") )
                        );
      access.setVerifier( 3, media::MediaVerifierRef(
                            new SimpleVerifier("cd3") )
                        );

      Pathname file1 = access.provideFile("/hello.text", 1 );
      Pathname file2 = access.provideFile("/hello.text", 2 );
      Pathname file3 = access.provideFile("/hello.text", 3 );
    }
    catch ( const Exception &e )
    {
      ERR << "ups! " << e.msg() << std::endl;
    }
    return 0;
}



