#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>
#include <cstdlib>

#include <signal.h>

using namespace zypp;
using namespace zypp::media;

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


bool       do_step = false;
int        do_quit = 0;

void quit(int)
{
    do_quit = 1;
}

void goon(int)
{
}

#define ONE_STEP(MSG) \
do { \
  DBG << "======================================" << std::endl; \
  DBG << "==>> " << MSG << std::endl; \
  DBG << "======================================" << std::endl; \
  if( do_step) { pause(); if( do_quit) exit(0); } \
} while(0);

int main(int argc, char *argv[])
{
  bool force_release_src = false;
  {
      struct sigaction sa;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags   = 0;
      sa.sa_handler = goon;
      sigaction(SIGINT,  &sa, NULL);
      sa.sa_handler = quit;
      sigaction(SIGTERM, &sa, NULL);

      std::cerr << "ARGS=" << argc << std::endl;
      for(int i=1; i < argc; i++)
      {
        if( std::string(argv[i]) == "-i")
          do_step = true;
        else
        if( std::string(argv[i]) == "-r")
          force_release_src = true;
      }
  }

  MediaVerifierRef verifier(
    new MyMediaVerifier(/* "SUSE-Linux-CORE-i386 9" */)
  );
  MediaManager     mm;
  media::MediaId   src;
  media::MediaId   iso;
  zypp::Url        src_url;
  zypp::Url        iso_url;

  src_url = "nfs://dist.suse.de/dist/install";

  iso_url = "iso:/";
  iso_url.setQueryParam("iso", "SUSE-10.1-Beta5/SUSE-Linux-10.1-beta5-i386-CD1.iso");
  iso_url.setQueryParam("url", src_url.asString());

  try
  {
    if( force_release_src)
    {
      ONE_STEP("SRC: open " + src_url.asString());
      src = mm.open(src_url);

      ONE_STEP("SRC: attach")
      mm.attach(src);
    }

    ONE_STEP("ISO: open " + iso_url.asString());
    iso = mm.open(iso_url);

    ONE_STEP("ISO: add verifier")
    mm.addVerifier(iso, verifier);

    ONE_STEP("ISO: attach")
    mm.attach(iso);

    ONE_STEP("provideFile(/INDEX.gz)")
    mm.provideFile(iso, Pathname("/INDEX.gz"));

    if( force_release_src)
    {
      try
      {
        ONE_STEP("SRC: release()")
        mm.release(src, true);
      }
      catch(const MediaException &e)
      {
        ZYPP_CAUGHT(e);
        ERR << "ONE: HUH? Eject hasn't worked?!" << std::endl;
      }
    }

    ONE_STEP("ISO: RELEASE")
    mm.release(iso);

    ONE_STEP("CLEANUP")
  }
  catch(const MediaException &e)
  {
    ERR << "Catched media exception..." << std::endl;
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
