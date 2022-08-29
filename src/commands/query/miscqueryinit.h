/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_QUERY_MISCQUERYINIT_INCLUDED
#define ZYPPER_COMMANDS_QUERY_MISCQUERYINIT_INCLUDED

#include "Zypper.h"
#include "global-settings.h"

#include <zypp/AutoDispose.h>

/**
 * Implements a shared initialization for a set of query commands
 * The Mixin requires the BASE to be a ZypperBaseCommand
 *
 * All positional arguments are converted into the repo filter list
 */

extern ZYpp::Ptr God;

template <typename T>
struct MiscQueryInitMixin : public T
{

  using T::T;

  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override {
    auto &repoFilter = InitRepoSettings::instanceNoConst()._repoFilter;
    for ( const auto & repo : positionalArgs_r ) {
      repoFilter.push_back( repo );	// convert arguments to '-r repo'
    }

    int code = this->defaultSystemSetup( zypper, ResetRepoManager | InitTarget | InitRepos | LoadResolvables );
    if ( code != ZYPPER_EXIT_OK )
      return code;

    return T::execute ( zypper, positionalArgs_r );

  }
};

#endif
