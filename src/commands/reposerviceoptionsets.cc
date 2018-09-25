#include "reposerviceoptionsets.h"
#include "utils/flags/flagtypes.h"
#include "utils/flags/zyppflags.h"
#include "src/repos.h"
#include "main.h"

#include "utils/getopt.h"
#include "Zypper.h"

using namespace zypp;

namespace {
ZyppFlags::Value PriorityType(unsigned &target, const boost::optional<unsigned> &defValue)
{
  return ZyppFlags::Value (
        [defValue]() -> boost::optional<std::string>{
          if(defValue) {
            return std::to_string(*defValue);
          } else
            return boost::optional<std::string>();
        },

        [&target]( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> &in ) {
          if (!in)
            ZYPP_THROW(ZyppFlags::MissingArgumentException(opt.name));

          int prio = -1;
          safe_lexical_cast( *in, prio ); // try to make an int out of the string

          if ( prio < 0 )
          {
            ZYPP_THROW(ZyppFlags::InvalidValueException(
                         opt.name,
                         *in,
                         str::Format(_("Invalid priority '%s'. Use a positive integer number. The greater the number, the lower the priority.")) % *in));
          }

          target = ( prio ? unsigned(prio) : RepoInfo::defaultPriority() );
        },
        "PRIORITY"
  );
}

ZyppFlags::Value GPGCheckType(RepoInfo::GpgCheck &target, RepoInfo::GpgCheck valueIfSeen)
{
  return ZyppFlags::Value (
        ZyppFlags::noDefaultValue,
        [&target, valueIfSeen]( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
          target = valueIfSeen;
        }
  );
}

}

RepoServiceCommonOptions::RepoServiceCommonOptions(OptCommandCtx ctx)
  : _cmdContext(ctx)
{ }

RepoServiceCommonOptions::RepoServiceCommonOptions(OptCommandCtx ctx, ZypperBaseCommand &parent)
  : BaseCommandOptionSet(parent),
    _cmdContext(ctx)
{ }

std::vector<ZyppFlags::CommandGroup> RepoServiceCommonOptions::options()
{
  return {
    {
      {
        {"name",   'n', ZyppFlags::RequiredArgument, ZyppFlags::StringType( &_name, boost::optional<const char *>(), "NAME"),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Set a descriptive name for the service.") : _("Set a descriptive name for the repository.")
        },
        {"enable", 'e', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enable, ZyppFlags::StoreTrue, TriBool( true ) ),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Enable a disabled service.") : _("Enable a disabled repository.")
        },
        {"disable", 'd', ZyppFlags::NoArgument, ZyppFlags::TriBoolType(_enable, ZyppFlags::StoreFalse, TriBool( false ) ),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Disable the service (but don't remove it).") : _("Disable the repository (but don't remove it).")
        },
        {"refresh", 'f', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enableAutoRefresh, ZyppFlags::StoreTrue, TriBool( true ) ),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Enable auto-refresh of the service.") : _("Enable auto-refresh of the repository.")
        },
        {"no-refresh", 'F', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enableAutoRefresh, ZyppFlags::StoreFalse, TriBool( false ) ),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Disable auto-refresh of the service.") : _("Disable auto-refresh of the repository.")
        }
      }
    }
  };
}

void RepoServiceCommonOptions::reset()
{
  _name.clear();
  _enable = indeterminate;
  _enableAutoRefresh = indeterminate;
}

void RepoServiceCommonOptions::fillFromCopts( Zypper &zypper )
{
  reset();
  parsed_opts::const_iterator it = zypper.cOpts().find("name");
  if ( it != zypper.cOpts().end() )
    _name = it->second.front();

  _enable = get_boolean_option( zypper, "enable", "disable" );
  _enableAutoRefresh = get_boolean_option( zypper, "refresh", "no-refresh" );
}

RepoServiceCommonSelectOptions::RepoServiceCommonSelectOptions(OptCommandCtx ctx)
  : _cmdContext(ctx)
{ }

RepoServiceCommonSelectOptions::RepoServiceCommonSelectOptions(OptCommandCtx ctx, ZypperBaseCommand &parent)
  : BaseCommandOptionSet(parent),
    _cmdContext(ctx)
{ }

std::vector<ZyppFlags::CommandGroup> RepoServiceCommonSelectOptions::options()
{
  return {{{
      {"all", 'a', ZyppFlags::NoArgument, ZyppFlags::BoolType( &_all, ZyppFlags::StoreTrue, _all),
              _cmdContext == OptCommandCtx::ServiceContext ? _("Apply changes to all services.") : _("Apply changes to all repositories.")
      },
      {"local", 'l', ZyppFlags::NoArgument, ZyppFlags::BoolType( &_local, ZyppFlags::StoreTrue, _local),
              _cmdContext == OptCommandCtx::ServiceContext ?  _("Apply changes to all local services.") : _("Apply changes to all local repositories.")
      },
      {"remote", 't', ZyppFlags::NoArgument, ZyppFlags::BoolType( &_remote, ZyppFlags::StoreTrue, _remote),
              _cmdContext == OptCommandCtx::ServiceContext ?  _("Apply changes to all remote services.") : _("Apply changes to all remote repositories.")
      },
      {"medium-type", 'm', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &_mediumTypes, "TYPE"),
              _cmdContext == OptCommandCtx::ServiceContext ?  _("Apply changes to services of specified type.") : _("Apply changes to repositories of specified type.")
      }
  }}};
}

void RepoServiceCommonSelectOptions::reset()
{
  _all    = false;
  _local  = false;
  _remote = false;
  _mediumTypes.clear();
}

void RepoServiceCommonSelectOptions::fillFromCopts(Zypper &zypper)
{
  reset();
  _all    = copts.count("all");
  _local  = copts.count("local");
  _remote = copts.count("remote");
  _mediumTypes.insert( _mediumTypes.begin(), copts["medium-type"].begin(), copts["medium-type"].end());
}


std::vector<ZyppFlags::CommandGroup> RepoProperties::options()
{
  return {{{
        { "priority", 'p', ZyppFlags::RequiredArgument, PriorityType( _priority, _priority ), _("Set priority of the repository.") },
        { "keep-packages", 'k', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _keepPackages, ZyppFlags::StoreTrue, TriBool( true ) ), _("Enable RPM files caching.") },
        { "no-keep-packages", 'K', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _keepPackages, ZyppFlags::StoreFalse, TriBool( false ) ), _("Disable RPM files caching.") },
        { "gpgcheck", 'g', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::On ), _("Enable GPG check for this repository.") },
        { "gpgcheck-strict", '\0', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::Strict ), _("Enable strict GPG check for this repository.") },
        { "gpgcheck-allow-unsigned", '\0', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::AllowUnsigned ), str::Format(_("Short hand for '%1%'.") ) % "--gpgcheck-allow-unsigned-repo --gpgcheck-allow-unsigned-package" },
        { "gpgcheck-allow-unsigned-repo", '\0', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::AllowUnsignedRepo ), _("Enable GPG check but allow the repository metadata to be unsigned.") },
        { "gpgcheck-allow-unsigned-package", '\0', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::AllowUnsignedPackage ), _("Enable GPG check but allow installing unsigned packages from this repository.") },
        { "no-gpgcheck", 'G', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::Off ), _("Disable GPG check for this repository.") },
        { "default-gpgcheck", '\0', ZyppFlags::NoArgument, GPGCheckType( _gpgCheck, RepoInfo::GpgCheck::Default ), _("Use the global GPG check setting defined in /etc/zypp/zypp.conf. This is the default.") }
      },{
        //conflicting arguments
        { "gpgcheck", "gpgcheck-strict", "gpgcheck-allow-unsigned", "gpgcheck-allow-unsigned-repo", "gpgcheck-allow-unsigned-package", "no-gpgcheck", "default-gpgcheck"}
      }}};
}

void RepoProperties::reset()
{
  _priority = 0U;
  _keepPackages = zypp::indeterminate;
  _gpgCheck = zypp::RepoInfo::GpgCheck::indeterminate;
}

void RepoProperties::fillFromCopts(Zypper &zypper)
{
  reset();
  _keepPackages = get_boolean_option( zypper, "keep-packages", "no-keep-packages" );
  _gpgCheck = cli::gpgCheck( zypper );
  _priority = priority_from_copts( zypper );
}


RSCommonListOptions::RSCommonListOptions(OptCommandCtx ctx)
  : _cmdContext ( ctx )
{ }

RSCommonListOptions::RSCommonListOptions(OptCommandCtx ctx, ZypperBaseCommand &parent)
  : BaseCommandOptionSet ( parent ),
    _cmdContext ( ctx )
{ }

std::vector<ZyppFlags::CommandGroup> RSCommonListOptions::options()
{

  std::vector<ZyppFlags::CommandGroup> opts;

  RSCommonListFlags showAllFlag = ListServiceShowAll;

  if ( _cmdContext  == OptCommandCtx::RepoContext ) {

    opts.push_back ( {{
            { "alias",  'a',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowAlias ),
                // translators: -a, --alias
              _("Show also repository alias.")},
            { "name",   'n',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowName ),
                // translators: -n, --name
              _("Show also repository name.")},
            { "refresh",   'r',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowRefresh ),
                // translators: -r, --refresh
              _("Show also the autorefresh flag.")}
          }} );

    showAllFlag = ListRepoShowAll;
  }

  opts.push_back(
    {{
      { "uri",  'u',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowURI ),
        // translators: -u, --uri
        _("Show also base URI of repositories.")},
      { "url", '\0',       ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BitFieldType( _flags, ShowURI ), "" },
      { "priority",   'p', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowPriority ),
        // translators: -p, --priority
        _("Show also repository priority.") },
      { "details",    'd', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, showAllFlag ),
        // translators: -d, --details
        _("Show more information like URI, priority, type.")  }
    }}
  );

  if ( _cmdContext == OptCommandCtx::ServiceContext ) {
    //@NOTE attention -r clashes with --refresh in case of merging the options
    opts.back().options.push_back(
      { "with-repos", 'r', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowWithRepos), _("Show also repositories belonging to the services.") }
    );
  } else {
    opts.back().options.push_back(
      { "service", 's', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowWithService),
        // translators: -s, --service
        _("Show also alias of parent service.") }
    );
  }

  opts.push_back({{
     { "show-enabled-only", 'E', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, ShowEnabledOnly ),
       // translators: -E, --show-enabled-only
       _("Show enabled repos only.") },
     { "sort-by-uri", 'U',       ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, RSCommonListFlags( ShowURI ) | SortByURI ),
       // translators: -U, --sort-by-uri
       _("Sort the list by URI.") },
     { "sort-by-name", 'N',      ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, SortByName ),
       // translators: -N, --sort-by-name
       _("Sort the list by name.") },
     { "sort-by-priority", 'P',  ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, RSCommonListFlags( ShowPriority ) | SortByPrio ),
       // translators: -P, --sort-by-priority
       _("Sort the list by repository priority.") }
  }});

  if ( _cmdContext  == OptCommandCtx::RepoContext ) {
    opts.back().options.push_back(
      { "sort-by-alias", 'A', ZyppFlags::NoArgument,  ZyppFlags::BitFieldType( _flags, RSCommonListFlags( ShowAlias ) | SortByAlias ),
        // translators: -A, --sort-by-alias
        _("Show also alias of parent service.") }
    );
  }

  return opts;
}

void RSCommonListOptions::reset()
{
  _flags.unsetFlag( _flags.all() );
}
