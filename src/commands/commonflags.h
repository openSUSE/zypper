/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED
#define ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED

#include "utils/flags/flagtypes.h"

/**
 * \file Contains all flags that are commonly used in multiple commands but which are too simple for a OptionSet
 */

namespace CommonFlags
{

  inline zypp::ZyppFlags::CommandOption detailsFlag ( bool &targetFlag ) {
    return {
      "details", '\0', zypp::ZyppFlags::NoArgument, zypp::ZyppFlags::BoolType( &targetFlag, zypp::ZyppFlags::StoreTrue, targetFlag ),
      // translators: --details
      _("Show the detailed installation summary.")
    };;
  }

  inline zypp::ZyppFlags::CommandOption resKindSetFlag ( std::set<ResKind> &target ) {
    return {
      "type", 't', zypp::ZyppFlags::RequiredArgument | ZyppFlags::Repeatable , zypp::ZyppFlags::KindSetType ( &target ) ,
       str::Format(_("Type of package (%1%).") ) % "package, patch, pattern, product"
    };
  }

  inline zypp::ZyppFlags::CommandOption replaceFilesFlag ( bool &targetFlag ) {
    return {
      "replacefiles", '\0', zypp::ZyppFlags::NoArgument, zypp::ZyppFlags::BoolType( &targetFlag, zypp::ZyppFlags::StoreTrue, targetFlag ),
      // translators: --replacefiles
      _("Install the packages even if they replace files from other, already installed, packages. Default is to treat file conflicts as an error. --download-as-needed disables the fileconflict check.")
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
}




#endif
