#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

using namespace zypp;
using namespace zypp::media;

/*
** Very basic example verifier.
**
** This one does not know anything about the product,
** it just checks if /media.<mediaNr> exists...
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
  isDesiredMedia(const MediaAccessRef &ref, MediaNr mediaNr)
  {
    DBG << "isDesiredMedia(): for media nr " << mediaNr << std::endl;

    if( !ref)
      DBG << "isDesiredMedia(): invalid media handle" << std::endl;

    std::list<std::string> lst;
    Pathname               dir("/media." + str::numstring(mediaNr));

    DBG << "isDesiredMedia(): checking " << dir.asString() << std::endl;

    // FIXME: check the product e.g. via /media.X/products as well,
    //        not only if the media nr is the correct one.
    try
    {
      if( ref)
        ref->dirInfo(lst, dir, false);
    }
    catch( ... )
    {}
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

  try
  {

    id = mm.open(zypp::Url("cd:"));

    mm.addVerifier( id, verifier);

    mm.attach(id);

    mm.provideFile(id, 1, Pathname("/directory.yast"));
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
