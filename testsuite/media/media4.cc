#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>
#include <zypp/PathInfo.h>
#include <zypp/TmpPath.h>
#include <zypp/ExternalProgram.h>

#include <string>
#include <list>
#include <iostream>
#include <cstdlib>

#include <signal.h>
#include <stdlib.h>

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

  MediaManager     mm;
  media::MediaId   id;
  zypp::Url        url;
  Pathname         dir("./suse/setup/descr");

  url = "cd:/";

  try
  {
    ONE_STEP("open " + url.asString());
    id = mm.open(url);

    ONE_STEP("attach")
    mm.attach(id);

    ONE_STEP("provideDirTree(" + dir.asString() + ")");
    mm.provideDirTree(id, Pathname(dir));

    ONE_STEP("Create a temporary dir");
    zypp::filesystem::TmpDir temp;

    ONE_STEP("Create a copy of " + dir.asString());
    zypp::filesystem::copy_dir(mm.localPath(id, dir), temp.path());

    std::string cmd("/bin/ls -lR ");
                cmd += temp.path().asString();

    ONE_STEP("Check the directory copy")
    system( cmd.c_str());

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
