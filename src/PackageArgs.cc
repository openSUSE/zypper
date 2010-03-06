/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file PackageArgs.cc
 * 
 */

#include <iostream>
#include "zypp/base/Logger.h"

#include "PackageArgs.h"
#include "Zypper.h"


using namespace std;


PackageArgs::PackageArgs()
  : zypper(*Zypper::instance())
{
  preprocess(zypper.arguments());
}

PackageArgs::PackageArgs(const vector<string> & args)
  : zypper(*Zypper::instance())
{
  preprocess(args);
}

void PackageArgs::preprocess(const vector<string> & args)
{
  vector<string>::size_type argc = args.size();

  string tmp;
  string arg;
  bool op = false;
  for(unsigned i = 0; i < argc; ++i)
  {
    tmp = args[i];

    if (op)
    {
      arg += tmp;
      op = false;
      tmp.clear();
    }
    // standalone operator
    else if (tmp == "=" || tmp == "==" || tmp == "<"
            || tmp == ">" || tmp == "<=" || tmp == ">=")
    {
      // not at the start or the end
      if (i && i < argc - 1)
        op = true;
    }
    // operator at the end of a random string, e.g. 'zypper='
    else if (tmp.find_last_of("=<>") == tmp.size() - 1 && i < argc - 1)
    {
      if (!arg.empty())
        _args.insert(arg);
      arg = tmp;
      op = true;
      continue;
    }
    // operator at the start of a random string e.g. '>=3.2.1'
    else if (i && tmp.find_first_of("=<>") == 0)
    {
      arg += tmp;
      tmp.clear();
      op = false;
    }

    if (op)
      arg += tmp;
    else
    {
      if (!arg.empty())
        _args.insert(arg);
      arg = tmp;
    }
  }

  if (!arg.empty())
    _args.insert(arg);

  DBG << "args received: ";
  copy(args.begin(), args.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl;

  DBG << "args compiled: ";
  copy(_args.begin(), _args.end(), ostream_iterator<string>(DBG, " "));
  DBG << endl;
}
