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
#include <iosfwd>

#include "zypp/Capability.h"

class Zypper;

struct PackageSpec
{
  PackageSpec() : modified(false) {}

  std::string orig_str;
  zypp::Capability parsed_cap;
  std::string repo_alias;
  bool modified;
};

/**
 * Only comparing parsed capabilities. Even though repository may be different,
 * if capability is the same, we must rule out one of them.
 */
struct PackageSpecCompare
{
  bool operator()(const PackageSpec & lhs, const PackageSpec & rhs) const
  {
    return lhs.parsed_cap < rhs.parsed_cap;
  };
};

class PackageArgs
{
public:
  typedef std::set<std::string> StringSet;
  typedef std::set<PackageSpec, PackageSpecCompare> PackageSpecSet;

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
  const PackageSpecSet & dos() const
  { return _dos; }
  /** Capabilities we don't want to install/upgrade or want to remove. */
  const PackageSpecSet & donts() const
  { return _donts; }

  bool empty() const
  { return dos().empty() && donts().empty(); }

protected:
  /** join arguments at comparison operators ('=', '>=', and the like) */
  void preprocess(const std::vector<std::string> & args);
  void argsToCaps(const zypp::ResKind & kind);

private:
  PackageArgs();

  Zypper & zypper;
  Options _opts;
  StringSet _args;
  PackageSpecSet _dos;
  PackageSpecSet _donts;
};


std::ostream & operator<<(std::ostream & out, const PackageSpec & spec);

#endif /* ZYPPER_PACKAGEARGS_H_ */
