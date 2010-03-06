/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

/** \file PackageArgs.h
 * 
 */

#ifndef ZYPPER_PACKAGEARGS_H_
#define ZYPPER_PACKAGEARGS_H_

#include <set>
#include <vector>
#include <string>

#include "zypp/Capability.h"

class Zypper;

class PackageArgs
{
  typedef std::set<std::string> StringSet;

public:
  PackageArgs();
  PackageArgs(const std::vector<std::string> & args);
  ~PackageArgs() {}

  StringSet asStringSet() const
  { return _args; }

protected:
  void preprocess(const std::vector<std::string> & args);

private:
  Zypper & zypper;
  StringSet _args;
};


#endif /* ZYPPER_PACKAGEARGS_H_ */
