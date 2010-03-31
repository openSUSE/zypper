/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/


/** \file SolverRequester.h
 * 
 */

#ifndef SOLVERREQUESTER_H_
#define SOLVERREQUESTER_H_

#include "PackageArgs.h"
#include "utils/misc.h" // for ResKindSet; might make sense to move this elsewhere

/**
 *
 */
class SolverRequester
{
public:
  struct Options
  {
    Options()
      : force(false)
      , force_by_cap(false)
      , force_by_name(false)
    {}

    bool force;
    bool force_by_cap;
    bool force_by_name;
    /** aliases of the repos from which the packages should be installed */
    std::list<std::string> from_repos;
  };

public:
  SolverRequester(const Options & opts = Options())
    : _opts(opts)
  {}

public:
  /** Request installation of specified objects. */
  void install(const PackageArgs & args);
  /** Request removal of specified objects. */
  void remove(const PackageArgs & args);
  /** Request update of specified objects. */
  void update(const PackageArgs & args);
  /** Update all objects of given kinds, wherever possible. */
  void update(const ResKindSet & kinds);

private:
  void install_remove(const PackageArgs & args, bool doinst);
  void install(const zypp::Capability & cap, const std::string & repoalias);
  void remove(const zypp::Capability & cap);
  void update(const zypp::Capability & cap, const std::string & repoalias);

  // TODO provide also public versions of these, taking optional Options and
  // reporting errors via an output argument.

  Options _opts;
};

#endif /* SOLVERREQUESTER_H_ */
