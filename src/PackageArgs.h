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
#include <utility>

#include "zypp/Capability.h"

class Zypper;

class PackageArgs
{
public:
  typedef std::set<std::string> StringSet;
  typedef std::pair<zypp::Capability, std::string> CapRepoPair;
  typedef std::set<CapRepoPair> CapRepoPairSet;

  struct Options
  {
    Options()
      : do_by_default(true)
    {}

    /** Whether to do (install/update) or dont (remove) by default
     *  (if +/- not specified in argument).*/
    bool do_by_default;
  };

public:
  /** Processes current Zypper::arguments() */
  PackageArgs(
      const zypp::ResKind & kind = zypp::ResKind::package,
      const Options & opts = Options());

  /** Takes arguments as a vector of strings */
  PackageArgs(
      const std::vector<std::string> & args,
      const zypp::ResKind & kind = zypp::ResKind::package,
      const Options & opts = Options());

  ~PackageArgs() {}

  const Options & options() const
  {return _opts; }

  const StringSet & asStringSet() const
  { return _args; }
  /** Capabilities we want to install/upgrade and don't want to remove, plus
   * associated requested repo */
  const CapRepoPairSet & doCaps() const
  { return _do_caps; }
  /** Capabilities we don't want to install/upgrade or want to remove. */
  const CapRepoPairSet & dontCaps() const
  { return _dont_caps; }

  bool empty() const
  { return doCaps().empty() && dontCaps().empty(); }

protected:
  /** join arguments at comparison operators ('=', '>=', and the like) */
  void preprocess(const std::vector<std::string> & args);
  void argsToCaps(const zypp::ResKind & kind);

private:
  PackageArgs();

  Zypper & zypper;
  Options _opts;
  StringSet _args;
  CapRepoPairSet _do_caps;
  CapRepoPairSet _dont_caps;
};


#endif /* ZYPPER_PACKAGEARGS_H_ */
