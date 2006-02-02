#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>

using namespace zypp;
using namespace zypp::media;

class MyMediaVerifier: public MediaVerifierBase
{
public:
  MyMediaVerifier()
    : MediaVerifierBase()
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
  MediaVerifierRef v(new MyMediaVerifier());
  MediaManager mm;
  media::MediaId      id;

  id = mm.open(zypp::Url("cd:"));
  mm.addVerifier( id, v);

  mm.attach(id);

  mm.provideFile(id, 1, Pathname("/directory.yast"));

  return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
