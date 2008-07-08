/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <sys/wait.h> //for wait()

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/TmpPath.h"

#include "../main.h"

#include "pager.h"

using namespace std;
using namespace zypp;

string pager_help_exit(const string & pager)
{
  string endfour = pager.substr(pager.size()-4,4);
  if (endfour == "less")
  {
    return str::form(_("Press '%c' to exit the pager."), 'q');
  }
  return string();
}

string pager_help_navigation(const string & pager)
{
  string endfour = pager.substr(pager.size()-4,4);
  if (endfour == "less")
  {
    return _("Use arrows or pgUp/pgDown keys to scroll the text by lines or pages.");
  }
  else if (endfour == "more")
  {
    return _("Use the Enter or Space key to scroll the text by lines or pages.");
  }
  return string();
}

//gets true if successfully display in pager
bool show_in_pager(const string & text)
{
  const char* envpager = getenv("PAGER");
  if (!envpager)
    envpager="more"; //basic posix default, must be in PATH
  string pager(envpager);
  filesystem::TmpFile tfile;
  string tpath = tfile.path().absolutename().c_str();
  ofstream os(tpath.c_str());

  string help = pager_help_navigation(pager);
  if (!help.empty())
    os << "(" << help << ")" << endl << endl;
  os << text;

  help = pager_help_exit(pager);
  if (!help.empty())
    os << endl << endl << "(" << help << ")";
  os.close();

  ostringstream cmdline;
  cmdline << pager <<" "<<tpath;

  switch(fork())
  {
  case -1:
    WAR << "fork failed" << endl;
    return false;

  case 0:
    execlp("sh","sh","-c",cmdline.str().c_str(),(char *)0);
    WAR << "exec failed with " << strerror(errno) << endl;
    exit(1); // cannot return false here, because here is another process
             // so only kill myself
             //! \todo FIXME proper exit code, message?

  default: 
    wait(0); //wait until pager end to disallow possibly terminal collision
  }

  return true;
}
