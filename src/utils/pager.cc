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
#include "Zypper.h"

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
  if (pager.rfind("less") == pager.size() - 5)
    return _("Use arrows or pgUp/pgDown keys to scroll the text by lines or pages.");
  else if (pager.rfind("more") == pager.size() - 5)
    return _("Use the Enter or Space key to scroll the text by lines or pages.");
  return string();
}

// ---------------------------------------------------------------------------

static bool show_in_pager(const string & pager, const Pathname & file)
{
  if (Zypper::instance()->globalOpts().non_interactive)
    return true;

  ostringstream cmdline;
  cmdline << "'" << pager << "' '" << file << "'";

  string errmsg;
  pid_t pid;
  switch(pid = fork())
  {
  case -1:
    WAR << "fork failed" << endl;
    return false;

  case 0:
    execlp("sh","sh","-c",cmdline.str().c_str(),(char *)0);
    WAR << "exec failed with " << strerror(errno) << endl;
    // exit, cannot return false here, because this is another process
    //! \todo FIXME different exit code + message
    exit(ZYPPER_EXIT_ERR_BUG);

  default:
    DBG << "Executed pager process (pid: " << pid << ")" << endl;

    // wait until pager exits
    int status = 0;
    int ret;
    do
    {
      ret = waitpid(pid, &status, 0);
    }
    while (ret == -1 && errno == EINTR);

    if (WIFEXITED (status))
    {
      status = WEXITSTATUS (status);
      if (status)
      {
        DBG << "Pid " << pid << " exited with status " << status << endl;
        return false;
      }
      else
        DBG << "Pid " << pid << " successfully completed" << endl;
    }
    else if (WIFSIGNALED (status))
    {
      status = WTERMSIG (status);
      WAR << "Pid " << pid << " was killed by signal " << status
          << " (" << strsignal(status);
      if (WCOREDUMP (status))
        WAR << ", core dumped";
      WAR << ")" << endl;
      return false;
    }
    else
    {
      ERR << "Pid " << pid << " exited with unknown error" << endl;
      return false;
    }
  }

  return true;
}

// ---------------------------------------------------------------------------

bool show_text_in_pager(const string & text, const string & intro)
{
  const char* envpager = ::getenv("PAGER");
  if (!envpager || ::strlen(envpager) == 0)
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
  const char* envpager = ::getenv("PAGER");
  if (!envpager || ::strlen(envpager) == 0)
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
