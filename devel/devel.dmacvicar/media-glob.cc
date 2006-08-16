#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

using namespace zypp;
using namespace zypp::media;
typedef std::list <std::string> stringlist;
/*
** Very basic example verifier.
**
** This one does not know anything about the product, it
** just checks if /media.1 (limited to 1st CD) exists...
*/
class MyMediaVerifier: public MediaVerifierBase
{
private:
  // std::string _productname;
public:
  MyMediaVerifier(/* std::string &productname */)
    : MediaVerifierBase()
    //, _productname(productname)
  {}

  virtual
  ~MyMediaVerifier()
  {}

  virtual bool
  isDesiredMedia(const MediaAccessRef &ref)
  {
    DBG << "isDesiredMedia(): for media nr 1 " << std::endl;

    if( !ref)
      DBG << "isDesiredMedia(): invalid media handle" << std::endl;

    std::list<std::string> lst;
    Pathname               dir("/media.1");

    DBG << "isDesiredMedia(): checking " << dir.asString() << std::endl;

    // check the product e.g. via /media.1/products as well...
    try
    {
      if( ref)
        ref->dirInfo(lst, dir, false);
    }
    catch(const zypp::Exception &e)
    {
      ZYPP_CAUGHT(e);
    }
    DBG << "isDesiredMedia(): media "
        << (lst.empty() ? "does not contain" : "contains")
        << " the " << dir.asString() << " directory."
        << std::endl;

    return !lst.empty();
  }
};

int main(void)
{
  MediaVerifierRef verifier(
    new MyMediaVerifier(/* "SUSE-Linux-CORE-i386 9" */)
  );
  MediaManager     mm;
  media::MediaId   id;
  media::MediaId   id2;
  try
  {

    //id = mm.open(zypp::Url("cd:/"), "");
    id = mm.open(zypp::Url("http://duncan.mac-vicar.com/photos/"), "");
    //mm.addVerifier( id, verifier);
    mm.attach(id);
    //mm.provideFile(id, Pathname("/directory.yast"));
    //mm.release(id);
    //mm.attach(id);
    //mm.provideFile(id, Pathname("/directory.yast"));

    //void dirInfo(MediaAccessId accessId, std::list<std::string> &retlist, const Pathname &dirname, bool dots = true) const;
    //stringlist retlist;
    //mm.dirInfo(id, retlist, "/");
    
    mm.doesFileExist(id, "bateria.jpg");
    
    mm.doesFileExist(id, "fdsfsd.jpg");
    
    id2 = mm.open( zypp::Url("ftp://ftp.kernel.org/pub/"), "");
    mm.attach(id2);
    
    mm.doesFileExist(id2, "README");
    mm.doesFileExist(id2, "notExists");
    
    /*
    std::cout << "DirInfo: " << retlist.size() << " objects" << std::endl;
    for (  stringlist::const_iterator it = retlist.begin(); it != retlist.end(); ++it)
    {
      std::cout << "item: " << *it << std::endl;
    }
    */
  }
  catch(const MediaException &e)
  {
    ZYPP_CAUGHT(e);
  }
  catch( ... )
  {
    // hmm...
    ERR << "Catched *unknown* exception" << std::endl;
  }

  return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
