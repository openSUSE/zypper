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
#include <iterator>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/TmpPath.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

#include "../main.h"

#include "pager.h"

using namespace std;
using namespace zypp;

// ---------------------------------------------------------------------------

static string pager_help_exit(const string & pager)
{
  string endfour = pager.substr(pager.size()-4,4);
  if (endfour == "less")
  {
    return str::form(_("Press '%c' to exit the pager."), 'q');
  }
  return string();
}

// ---------------------------------------------------------------------------

static string pager_help_navigation(const string & pager)
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

// ---------------------------------------------------------------------------

static bool show_in_pager(const string & pager, const Pathname & file)
{
  ostringstream cmdline;
  cmdline << "'" << pager << "' '" << file << "'";

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

// ---------------------------------------------------------------------------

bool show_text_in_pager(const string & text, const string & intro)
{
  const char* envpager = getenv("PAGER");
  if (!envpager)
    envpager = "more"; // basic posix default, must be in PATH
  string pager(envpager);

  filesystem::TmpFile tfile;
  string tpath = tfile.path().absolutename().asString();
  ofstream os(tpath.c_str());

  // intro
  if (!intro.empty())
    os << intro << endl;

  // navigaion hint
  string help = pager_help_navigation(pager);
  if (!help.empty())
    os << "(" << help << ")" << endl << endl;

  // the text
  os << text;

  // exit hint
  help = pager_help_exit(pager);
  if (!help.empty())
    os << endl << endl << "(" << help << ")";
  os.close();

  return show_in_pager(pager, tfile);
}

// ---------------------------------------------------------------------------

bool show_file_in_pager(const Pathname & file, const string & intro)
{
  const char* envpager = getenv("PAGER");
  if (!envpager)
    envpager = "more"; // basic posix default, must be in PATH
  string pager(envpager);

  filesystem::TmpFile tfile;
  string tpath = tfile.path().absolutename().asString();
  ofstream os(tpath.c_str());

  // intro
  if (!intro.empty())
    os << intro << endl;

  // navigaion hint
  string help = pager_help_navigation(pager);
  if (!help.empty())
    os << "(" << help << ")" << endl << endl;

  // the text
  ifstream is(file.asString().c_str());
  if (is.good())
    os << is.rdbuf();
  else
  {
    cerr << "ERR reading the file" << endl;
    is.close();
    return false;
  }
  is.close();

  // exit hint
  help = pager_help_exit(pager);
  if (!help.empty())
    os << endl << endl << "(" << help << ")";
  os.close();

  return show_in_pager(pager, tfile);
}

// vim: set ts=2 sts=2 sw=2 et ai:
