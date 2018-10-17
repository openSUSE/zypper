/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <map>
#include <functional>

#include <zypp/base/NamedValue.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>

#include "main.h"
#include "Command.h"

#include "commands/locks.h"
#include "commands/services.h"
#include "commands/repos.h"
#include "commands/ps.h"
#include "commands/needs-rebooting.h"
#include "commands/utils.h"

using namespace zypp;

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

///////////////////////////////////////////////////////////////////
namespace
{

  template<typename T>
  ZypperBaseCommandPtr commandFactory ( const std::vector<std::string> &aliases_r )
  {
    return std::make_shared<T> ( aliases_r );
  }

  struct CommandFactory {
    std::vector<std::string> aliases;
    std::function<ZypperBaseCommandPtr ( const std::vector<std::string> & )> constructor;

    ZypperBaseCommandPtr operator ()()
    {
      return constructor( aliases );
    }

    template <typename T>
    constexpr static CommandFactory make ( const std::vector<std::string> &aliases_r )
    {
      return CommandFactory { aliases_r, commandFactory<T>};
    }
  };

  //@TODO hack for now, this should be migrated to be part of Zypper class directly instead of a
  //singleton
  static std::map< ZypperCommand::Command, CommandFactory > &newStyleCommands ()
  {
    static std::map< ZypperCommand::Command,  CommandFactory> table {
      { ZypperCommand::LIST_LOCKS_e,  CommandFactory::make<ListLocksCmd>( { "locks",       "ll", "lock-list"          }) },
      { ZypperCommand::ADD_LOCK_e,    CommandFactory::make<AddLocksCmd>({ "addlock",     "al", "lock-add",     "la" }) },
      { ZypperCommand::REMOVE_LOCK_e, CommandFactory::make<RemoveLocksCmd>({ "removelock",  "rl", "lock-delete" , "ld" }) },
      { ZypperCommand::CLEAN_LOCKS_e, CommandFactory::make<CleanLocksCmd> ({ "cleanlocks" , "cl", "lock-clean"         }) },

      { ZypperCommand::LIST_SERVICES_e, CommandFactory::make<ListServicesCmd>( { "services", "ls", "service-list", "sl" } ) },
      { ZypperCommand::REFRESH_SERVICES_e, CommandFactory::make<RefreshServicesCmd>( { "refresh-services", "refs" } ) },
      { ZypperCommand::MODIFY_SERVICE_e, CommandFactory::make<ModifyServiceCmd>( { "modifyservice", "ms" } ) }, //<<
      { ZypperCommand::REMOVE_SERVICE_e, CommandFactory::make<RemoveServiceCmd>( { "removeservice", "rs", "service-delete", "sd" } ) },
      { ZypperCommand::ADD_SERVICE_e, CommandFactory::make<AddServiceCmd>( { "addservice", "as", "service-add", "sa" } ) },

      { ZypperCommand::LIST_REPOS_e, CommandFactory::make<ListReposCmd>( {"repos", "lr", "catalogs","ca"} ) },
      { ZypperCommand::ADD_REPO_e, CommandFactory::make<AddRepoCmd>( { "addrepo", "ar" } ) },
      { ZypperCommand::REMOVE_REPO_e, CommandFactory::make<RemoveRepoCmd>( { "removerepo", "rr" } ) },
      { ZypperCommand::RENAME_REPO_e, CommandFactory::make<RenameRepoCmd>( { "renamerepo", "nr" } ) },
      { ZypperCommand::MODIFY_REPO_e, CommandFactory::make<ModifyRepoCmd>( { "modifyrepo", "mr" } ) },
      { ZypperCommand::REFRESH_e, CommandFactory::make<RefreshRepoCmd>( { "refresh", "ref" } ) },
      { ZypperCommand::CLEAN_e, CommandFactory::make<CleanRepoCmd>( { "clean", "cc", "clean-cache", "you-clean-cache", "yc" } ) },

      { ZypperCommand::PS_e, CommandFactory::make<PSCommand>( { "ps" }) },
      { ZypperCommand::NEEDS_REBOOTING_e, CommandFactory::make<NeedsRebootingCmd>( { "needs-rebooting" }) },
      { ZypperCommand::TARGET_OS_e, CommandFactory::make<TargetOSCmd>( { "targetos", "tos" } ) },
      { ZypperCommand::VERSION_CMP_e, CommandFactory::make<VersionCompareCmd>( { "versioncmp", "vcmp" } ) },
      { ZypperCommand::LICENSES_e, CommandFactory::make<LicensesCmd>( { "licenses" } ) }
    };
    return table;
  }

  static NamedValue<ZypperCommand::Command> & cmdTable()
  {
    static NamedValue<ZypperCommand::Command> _table;
    if ( _table.empty() )
    {
#define _t(C) _table( ZypperCommand::C )
      _t( NONE_e )		| "NONE"		| "none" | "";
      _t( SUBCOMMAND_e)		| "subcommand";

      //_t( ADD_SERVICE_e )	| "addservice"		| "as" | "service-add" | "sa";
      //_t( REMOVE_SERVICE_e )	| "removeservice"	| "rs" | "service-delete" | "sd";
      //_t( MODIFY_SERVICE_e )	| "modifyservice"	| "ms";
      //_t( LIST_SERVICES_e )	| "services"		| "ls" | "service-list" | "sl";
      //_t( REFRESH_SERVICES_e )	| "refresh-services"	| "refs";

      //_t( ADD_REPO_e )		| "addrepo" 		| "ar";
      //_t( REMOVE_REPO_e )	| "removerepo"		| "rr";
      //_t( RENAME_REPO_e )	| "renamerepo"		| "nr";
      //_t( MODIFY_REPO_e )	| "modifyrepo"		| "mr";
      //_t( LIST_REPOS_e )	| "repos"		| "lr" | "catalogs" | "ca";
      //_t( REFRESH_e )		| "refresh"		| "ref";
      //_t( CLEAN_e )		| "clean"		| "cc" | "clean-cache" | "you-clean-cache" | "yc";

      _t( INSTALL_e )		| "install"		| "in";
      _t( REMOVE_e )		| "remove"		| "rm";
      _t( SRC_INSTALL_e )	| "source-install"	| "si";
      _t( VERIFY_e )		| "verify"		| "ve";
      _t( INSTALL_NEW_RECOMMENDS_e )| "install-new-recommends" | "inr";

      _t( UPDATE_e )		| "update"		| "up";
      _t( LIST_UPDATES_e )	| "list-updates"	| "lu";
      _t( PATCH_e )		| "patch";
      _t( LIST_PATCHES_e )	| "list-patches"	| "lp";
      _t( PATCH_CHECK_e )	| "patch-check"		| "pchk";
      _t( DIST_UPGRADE_e )	| "dist-upgrade"	| "dup";

      _t( SEARCH_e )		| "search"		| "se";
      _t( INFO_e )		| "info"		| "if";
      _t( PACKAGES_e )		| "packages"		| "pa" | "pkg";
      _t( PATCHES_e )		| "patches"		| "pch";
      _t( PATTERNS_e )		| "patterns"		| "pt";
      _t( PRODUCTS_e )		| "products"		| "pd";

      _t( WHAT_PROVIDES_e )	| "what-provides"	| "wp";
      //_t( WHAT_REQUIRES_e )	| "what-requires"	| "wr";
      //_t( WHAT_CONFLICTS_e )	| "what-conflicts"	| "wc";

      // _t( ADD_LOCK_e )		| "addlock"		| "al" | "lock-add" | "la";
      // _t( REMOVE_LOCK_e )	| "removelock"		| "rl" | "lock-delete" | "ld";
      // _t( LIST_LOCKS_e )	| "locks"		| "ll" | "lock-list";
      // _t( CLEAN_LOCKS_e )	| "cleanlocks"		| "cl" | "lock-clean";

      //_t( TARGET_OS_e )		| "targetos"		| "tos";
      //_t( VERSION_CMP_e )	| "versioncmp"		| "vcmp";
      //_t( LICENSES_e )		| "licenses";
      // _t( PS_e )		| "ps";
      _t( DOWNLOAD_e )		| "download";
      _t( SOURCE_DOWNLOAD_e )	| "source-download";

      _t( HELP_e )		| "help"		| "?";
      _t( SHELL_e )		| "shell"		| "sh";
      _t( SHELL_QUIT_e )	| "quit"		| "exit" | "\004";
      _t( MOO_e )		| "moo";

      _t( CONFIGTEST_e)		|  "configtest";

      _t( RUG_PATCH_INFO_e )	| "patch-info";
      _t( RUG_PATTERN_INFO_e )	| "pattern-info";
      _t( RUG_PRODUCT_INFO_e )	| "product-info";
      _t( RUG_PATCH_SEARCH_e )	| "patch-search" | "pse";
      _t( RUG_PING_e )		| "ping";
#undef _t

      // patch the table to contain all new style commands
      for ( const auto &cmd : newStyleCommands() ) {
        auto entry = _table(cmd.first);
        for ( const std::string &alias : cmd.second.aliases)
          entry | alias;
      }
    }
    return _table;
  }
} // namespace
///////////////////////////////////////////////////////////////////

#define DEF_ZYPPER_COMMAND(C) const ZypperCommand ZypperCommand::C( ZypperCommand::C##_e )
DEF_ZYPPER_COMMAND( NONE );
DEF_ZYPPER_COMMAND( SUBCOMMAND );

DEF_ZYPPER_COMMAND( ADD_SERVICE );
DEF_ZYPPER_COMMAND( REMOVE_SERVICE );
DEF_ZYPPER_COMMAND( MODIFY_SERVICE );
DEF_ZYPPER_COMMAND( LIST_SERVICES );
DEF_ZYPPER_COMMAND( REFRESH_SERVICES );

DEF_ZYPPER_COMMAND( ADD_REPO );
DEF_ZYPPER_COMMAND( REMOVE_REPO );
DEF_ZYPPER_COMMAND( RENAME_REPO );
DEF_ZYPPER_COMMAND( MODIFY_REPO );
DEF_ZYPPER_COMMAND( LIST_REPOS );
DEF_ZYPPER_COMMAND( REFRESH );
DEF_ZYPPER_COMMAND( CLEAN );

DEF_ZYPPER_COMMAND( INSTALL );
DEF_ZYPPER_COMMAND( REMOVE );
DEF_ZYPPER_COMMAND( SRC_INSTALL );
DEF_ZYPPER_COMMAND( VERIFY );
DEF_ZYPPER_COMMAND( INSTALL_NEW_RECOMMENDS );

DEF_ZYPPER_COMMAND( UPDATE );
DEF_ZYPPER_COMMAND( LIST_UPDATES );
DEF_ZYPPER_COMMAND( PATCH );
DEF_ZYPPER_COMMAND( LIST_PATCHES );
DEF_ZYPPER_COMMAND( PATCH_CHECK );
DEF_ZYPPER_COMMAND( DIST_UPGRADE );

DEF_ZYPPER_COMMAND( SEARCH );
DEF_ZYPPER_COMMAND( INFO );
DEF_ZYPPER_COMMAND( PACKAGES );
DEF_ZYPPER_COMMAND( PATCHES );
DEF_ZYPPER_COMMAND( PATTERNS );
DEF_ZYPPER_COMMAND( PRODUCTS );

DEF_ZYPPER_COMMAND( WHAT_PROVIDES );
//DEF_ZYPPER_COMMAND( WHAT_REQUIRES );
//DEF_ZYPPER_COMMAND( WHAT_CONFLICTS );

DEF_ZYPPER_COMMAND( ADD_LOCK );
DEF_ZYPPER_COMMAND( REMOVE_LOCK );
DEF_ZYPPER_COMMAND( LIST_LOCKS );
DEF_ZYPPER_COMMAND( CLEAN_LOCKS );

DEF_ZYPPER_COMMAND( TARGET_OS );
DEF_ZYPPER_COMMAND( VERSION_CMP );
DEF_ZYPPER_COMMAND( LICENSES );
DEF_ZYPPER_COMMAND( PS );
DEF_ZYPPER_COMMAND( DOWNLOAD );
DEF_ZYPPER_COMMAND( SOURCE_DOWNLOAD );

DEF_ZYPPER_COMMAND( HELP );
DEF_ZYPPER_COMMAND( SHELL );
DEF_ZYPPER_COMMAND( SHELL_QUIT );
DEF_ZYPPER_COMMAND( MOO );

DEF_ZYPPER_COMMAND( RUG_PATCH_INFO );
DEF_ZYPPER_COMMAND( RUG_PATTERN_INFO );
DEF_ZYPPER_COMMAND( RUG_PRODUCT_INFO );
DEF_ZYPPER_COMMAND( RUG_PATCH_SEARCH );
DEF_ZYPPER_COMMAND( RUG_PING );

DEF_ZYPPER_COMMAND( NEEDS_REBOOTING );

#undef DEF_ZYPPER_COMMAND
///////////////////////////////////////////////////////////////////

ZypperCommand::ZypperCommand(ZypperCommand::Command command) : _command(command)
{
  //set the command object if the passed enum represents a new style cmd
  auto &newCmds = newStyleCommands();
  if ( newCmds.find( _command ) != newCmds.end() ) {
    _newStyleCmdObj = newCmds[_command]();
  }
}

ZypperCommand::ZypperCommand( const std::string & strval_r )
  : ZypperCommand( parse(strval_r ) )
{ }

ZypperCommand::Command ZypperCommand::parse( const std::string & strval_r ) const
{
  ZypperCommand::Command cmd = SUBCOMMAND_e;	// Exception if not true
  if ( ! cmdTable().getValue( strval_r, cmd ) )
  {
    bool isSubcommand( const std::string & strval_r );	// in subcommand.cc

    if ( ! isSubcommand( strval_r ) )
    {
      ZYPP_THROW( Exception( str::form(_("Unknown command '%s'"), strval_r.c_str() ) ) );
    }
  }
  return cmd;
}

const std::string & ZypperCommand::asString() const
{ return cmdTable().getName( _command ); }

ZypperBaseCommandPtr ZypperCommand::commandObject() const
{
  return _newStyleCmdObj;
}
