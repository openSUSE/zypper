/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED
#define ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED

#include "utils/flags/flagtypes.h"
#include <zypp/ZConfig.h>

/**
 * \file Contains all flags that are commonly used in multiple commands but which are too simple for a OptionSet
 */

namespace CommonFlags
{

  inline zypp::ZyppFlags::CommandOption detailsFlag ( bool &targetFlag, char shortFlag = '\0',  std::string help = std::string()  ) {
    return {
      "details", shortFlag, zypp::ZyppFlags::NoArgument, zypp::ZyppFlags::BoolType( &targetFlag, zypp::ZyppFlags::StoreTrue, targetFlag ),
      // translators: --details
      help.empty() ? _("Show the detailed installation summary.") : help
    };
  }

  inline zypp::ZyppFlags::CommandOption resKindSetFlag ( std::set<ResKind> &target, std::string help = std::string() ) {
    return {
      "type", 't', zypp::ZyppFlags::RequiredArgument | ZyppFlags::Repeatable , zypp::ZyppFlags::KindSetType ( &target ) ,
       help.empty() ? std::string( str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product" ) : help
    };
  }

  inline zypp::ZyppFlags::CommandOption bestEffortUpdateFlag ( bool &targetFlag ) {
    return {
      "best-effort", '\0', zypp::ZyppFlags::NoArgument, zypp::ZyppFlags::BoolType( &targetFlag, zypp::ZyppFlags::StoreTrue, targetFlag ),
      // translators: --best-effort
      _("Do a 'best effort' approach to update. Updates to a lower than the latest version are also acceptable.")
    };
  }

  inline zypp::ZyppFlags::CommandOption updateStackOnlyFlag ( bool &targetFlag ) {
    return {
      "updatestack-only", '\0', zypp::ZyppFlags::NoArgument, zypp::ZyppFlags::BoolType( &targetFlag, zypp::ZyppFlags::StoreTrue, targetFlag ),
      _("Consider only patches which affect the package management itself.")
    };
  }

  inline zypp::ZyppFlags::CommandOption downloadMaxRetriesFlag () {
    return {
      "download-max-retries",  '\0', ZyppFlags::RequiredArgument, ZyppFlags::CallbackVal( []( const auto& opt, const boost::optional<std::string> &in ){
          ZConfig::instance().set_download_max_silent_tries(ZyppFlags::argValueConvert<int>(opt, in));
    }, ARG_INTEGER ), _("Number of times zypp should attempt a download in case it fails.") };
  }

  inline zypp::ZyppFlags::CommandOption downloadRetryWaitTimeFlag () {
    return {
      "download-retry-wait-time",  '\0', ZyppFlags::RequiredArgument, ZyppFlags::CallbackVal( []( const auto& opt, const boost::optional<std::string> &in ){
          ZConfig::instance().set_download_retry_wait_time(ZyppFlags::argValueConvert<int>(opt, in));
    }, ARG_INTEGER ), _("Time in seconds zypp should wait before retrying a failed download.") };
  }
}




#endif
