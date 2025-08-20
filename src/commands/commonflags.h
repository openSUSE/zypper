/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED
#define ZYPPER_COMMANDS_COMMONFLAGS_INCLUDED

#include "utils/flags/flagtypes.h"
#include "utils/misc.h"
#include "Zypper.h"

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

  static inline ZyppFlags::CommandOption idsOnlyFlag( bool &target, std::string &&kind ) {
    return {"ids-only", '\0', ZyppFlags::NoArgument,
      std::move( ZyppFlags::BoolType(&target, ZyppFlags::StoreTrue).after(
        [&target]() {
          auto &out = Zypper::instance().out();
          if (out.type() == Out::TYPE_XML)
            target = false;
          out.setVerbosity( Out::QUIET );
        }) ),
      // translators: --name-only
      str::Format(_("Just print %s ids.")) % kind
    };
  }

  static inline ZyppFlags::CommandOption idsOnlyFlag( bool &target, const ResKind & kind ) {
    // Print 'package' in the help, instead of "resolvables" as that's more consistent with other help messages
    // (We still pass ResKind::nokind so we can easilly change this later)
    auto &display_kind = kind == ResKind::nokind ? ResKind::package : kind;
    return idsOnlyFlag( target,  kind_to_string_localized( display_kind, 1 ));
  }
}




#endif
