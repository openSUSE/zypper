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
  {
      struct sigaction sa;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags   = 0;
      sa.sa_handler = goon;
      sigaction(SIGINT,  &sa, NULL);
      sa.sa_handler = quit;
      sigaction(SIGTERM, &sa, NULL);

      if( argc > 1 && std::string(argv[1]) == "-i")
        do_step = true;
  }

  MediaVerifierRef verifier(
    new MyMediaVerifier(/* "SUSE-Linux-CORE-i386 9" */)
  );
  MediaManager     mm;
  media::MediaId   one;
  media::MediaId   two;
  zypp::Url        url;

  url = "cd:";

  try
  {
    ONE_STEP("ONE: open " + url.asString());
    one = mm.open(url);

    ONE_STEP("TWO: open " + url.asString());
    two = mm.open(url);


    ONE_STEP("ONE: add verifier")
    mm.addVerifier( one, verifier);

    ONE_STEP("TWO: add verifier")
    mm.addVerifier( two, verifier);


    ONE_STEP("ONE: attach")
    mm.attach(one);

    ONE_STEP("ONE: provideFile(/INDEX.gz)")
    mm.provideFile(one, Pathname("/INDEX.gz"));

    ONE_STEP("TWO: attach")
    mm.attach(two);


    ONE_STEP("ONE: provideFile(/content)")
    mm.provideFile(one, Pathname("/content"));

    ONE_STEP("TWO: provideFile(/INDEX.gz)")
    mm.provideFile(two, Pathname("/INDEX.gz"));


    try
    {
      ONE_STEP("ONE: release()")
      mm.release(one, true);
    }
    catch(const MediaException &e)
    {
      ZYPP_CAUGHT(e);
      DBG << "ONE: OK, EXPECTED IT (try to eject shared media)" << std::endl;
    }

    ONE_STEP("ONE: release()")
    mm.release(one, false);


    try {
      ONE_STEP("ONE: provideFile(/content)")
      mm.provideFile(one, Pathname("/content"));
    }
    catch(const MediaException &e)
    {
      ZYPP_CAUGHT(e);
      DBG << "ONE: OK, EXPECTED IT (released)" << std::endl;
    }

    ONE_STEP("TWO: provideFile(/ls-lR.gz)")
    mm.provideFile(two, Pathname("/ls-lR.gz"));

    ONE_STEP("TWO: release()")
    mm.release(two, false);

    ONE_STEP("ONE: REATTACH IT")
    mm.attach(one);

    ONE_STEP("ONE: provideFile(/INDEX.gz)")
    mm.provideFile(one, Pathname("/INDEX.gz"));

    ONE_STEP("CLEANUP")
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
