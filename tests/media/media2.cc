#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include <string>
#include <list>
#include <iostream>
#include <cstdlib>

#include <signal.h>

#include "mymediaverifier.h"

using namespace zypp;
using namespace zypp::media;


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

  url = "cd:/";

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
      ERR << "ONE: HUH? Eject hasn't worked?!" << std::endl;
    }

    try {
      ONE_STEP("ONE: provideFile(/content)")
      mm.provideFile(one, Pathname("/content"));
    }
    catch(const MediaException &e)
    {
      ZYPP_CAUGHT(e);
      DBG << "ONE: OK, EXPECTED IT (released)" << std::endl;
    }

    try {
      ONE_STEP("TWO: provideFile(/ls-lR.gz)")
      mm.provideFile(two, Pathname("/ls-lR.gz"));
    }
    catch(const MediaException &e)
    {
      ZYPP_CAUGHT(e);
      DBG << "TWO: OK, EXPECTED IT (released)" << std::endl;
    }

    ONE_STEP("TWO: (RE)ATTACH IT")
    mm.attach(two);

    ONE_STEP("TWO: provideFile(/INDEX.gz)")
    mm.provideFile(two, Pathname("/INDEX.gz"));

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
