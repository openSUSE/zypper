/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "selectpatchoptionset.h"
#include "utils/flags/flagtypes.h"


SelectPatchOptionSet::SelectPatchOptionSet(ZypperBaseCommand &parent, SelectPatchOptionSet::AnyTypeMode mode_r) :
  BaseCommandOptionSet (
    parent
  ),
  _mode ( mode_r )
{ }

std::vector<zypp::ZyppFlags::CommandGroup> SelectPatchOptionSet::options()
{
  zypp::ZyppFlags::ArgFlags reqOrOpt = _mode == EnableAnyType ? ZyppFlags::OptionalArgument : ZyppFlags::RequiredArgument;
  zypp::ZyppFlags::CommandGroup grp({
    { "category", 'g', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::GenericContainerType( _select._requestedPatchCategories,  ARG_CATEGORY, "," ),
          // translators: -g, --category <CATEGORY>
          _("Select patches with the specified category.")
    },
    { "severity", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::GenericContainerType( _select._requestedPatchSeverity,  ARG_SEVERITY, "," ),
          // translators: --severity <SEVERITY>
          _("Select patches with the specified severity.")
    },
    { "date", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::GenericContainerType( _select._requestedPatchDates,  "YYYY-MM-DD"),
          // translators: --date <YYYY-MM-DD>
          _("Select patches issued up to, but not including, the specified date")
    },
    { "bugzilla", 'b', reqOrOpt | ZyppFlags::Repeatable, ZyppFlags::IssueSetType( _select._requestedIssues, "bugzilla", "#" ),
          // translators: -b, --bugzilla
          _("Select applicable patches for the specified Bugzilla issues.")
    },
    { "bz", '\0',  reqOrOpt | ZyppFlags::Repeatable | ZyppFlags::Hidden, ZyppFlags::IssueSetType( _select._requestedIssues, "bugzilla", "#" ), "" },
    { "cve", '\0', reqOrOpt | ZyppFlags::Repeatable, ZyppFlags::IssueSetType( _select._requestedIssues, "cve", "#" ),
          // translators: --cve
          _("Select applicable patches for the specified CVE issues.")
    }
  });

  //enable the --issue switch
  if ( _mode == EnableAnyType ) {
    grp << zypp::ZyppFlags::CommandOption {
           "issue", '\0', reqOrOpt | ZyppFlags::Repeatable, ZyppFlags::IssueSetType( _select._requestedIssues, ""/*any*/, ARG_STRING ),
            // translators: --cve
            _("Select issues matching the specified string.")
    };
  }

  return {{
      grp
  }};
}

void SelectPatchOptionSet::reset()
{
  _select._requestedIssues.clear();
  _select._requestedPatchCategories.clear();
  _select._requestedPatchSeverity.clear();
  _select._requestedPatchDates.clear();
}
