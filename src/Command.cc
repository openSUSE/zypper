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
#include "commands/query.h"
#include "commands/installremove.h"
#include "commands/sourceinstall.h"
#include "commands/distupgrade.h"
#include "commands/inrverify.h"
#include "commands/patch.h"
#include "commands/update.h"
#include "commands/patchcheck.h"
#include "commands/listpatches.h"
#include "commands/listupdates.h"
#include "commands/search/search.h"
#include "commands/nullcommands.h"
#include "commands/configtest.h"
#include "commands/shell.h"
#include "commands/help.h"

using namespace zypp;

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

///////////////////////////////////////////////////////////////////
namespace
{
  template < typename T, typename ... Args >
  ZypperCommand::CmdDesc makeCmd ( ZypperCommand::Command comm, const std::string &category, std::vector<std::string> &&aliases, Args&&... args ) {
    return std::make_tuple(comm, category, aliases,
      ZypperCommand::CmdFactory( [ aliases, args... ]() {
        return std::make_shared<T>( aliases, args... );
      })
    );
  }

  ZypperBaseCommandPtr voidCmd ( )
  {
    ZYPP_THROW( ExitRequestException( str::form(_("Invalid command") ) ) );
    return ZypperBaseCommandPtr();
  }


  //@TODO hack for now, this should be migrated to be part of Zypper class directly instead of a
  //singleton
  static std::vector< ZypperCommand::CmdDesc > &newStyleCommands ()
  {
    /// The commands here are in a specific order, do not change that unless you know what you are doing
    /// Every command that has a category string will start a new category in the help output, all following
    /// commands are in the last started category until a new one is found.
    /// All commands that follow the HIDDEN category will not be shown in the help output.
    static std::vector< ZypperCommand::CmdDesc > commands {

      makeCmd<HelpCmd> ( ZypperCommand::HELP_e, std::string(), { "help", "?" } ),
      makeCmd<ShellCmd>( ZypperCommand::SHELL_e, std::string(), { "shell", "sh" } ),

      makeCmd<ListReposCmd> ( ZypperCommand::LIST_REPOS_e, _("Repository Management:"), {"repos", "lr", "catalogs","ca"} ),
      makeCmd<AddRepoCmd>   ( ZypperCommand::ADD_REPO_e , std::string() , { "addrepo", "ar" }),
      makeCmd<RemoveRepoCmd> ( ZypperCommand::REMOVE_REPO_e , std::string(), { "removerepo", "rr" } ),
      makeCmd<RenameRepoCmd> ( ZypperCommand::RENAME_REPO_e , std::string(), { "renamerepo", "nr" } ),
      makeCmd<ModifyRepoCmd> ( ZypperCommand::MODIFY_REPO_e , std::string(), { "modifyrepo", "mr" } ),
      makeCmd<RefreshRepoCmd> ( ZypperCommand::REFRESH_e , std::string(), { "refresh", "ref" } ),
      makeCmd<CleanRepoCmd> ( ZypperCommand::CLEAN_e , std::string(), { "clean", "cc", "clean-cache", "you-clean-cache", "yc" } ),

      makeCmd<ListServicesCmd> ( ZypperCommand::LIST_SERVICES_e , _("Service Management:"), { "services", "ls", "service-list", "sl" } ),
      makeCmd<AddServiceCmd> ( ZypperCommand::ADD_SERVICE_e , std::string(),  { "addservice", "as", "service-add", "sa" } ),
      makeCmd<ModifyServiceCmd> ( ZypperCommand::MODIFY_SERVICE_e , std::string(), { "modifyservice", "ms" } ),
      makeCmd<RemoveServiceCmd> ( ZypperCommand::REMOVE_SERVICE_e , std::string(), { "removeservice", "rs", "service-delete", "sd" } ),
      makeCmd<RefreshServicesCmd> ( ZypperCommand::REFRESH_SERVICES_e , std::string(), { "refresh-services", "refs" } ),

      makeCmd<InstallCmd> ( ZypperCommand::INSTALL_e , _("Software Management:"), { "install", "in" } ),
      makeCmd<RemoveCmd> ( ZypperCommand::REMOVE_e , std::string(), { "remove", "rm" } ),
      makeCmd<InrVerifyCmd> ( ZypperCommand::VERIFY_e , std::string(), { "verify", "ve" }, InrVerifyCmd::Mode::Verify ),
      makeCmd<SourceInstallCmd> ( ZypperCommand::SRC_INSTALL_e , std::string(), { "source-install", "si" } ),
      makeCmd<InrVerifyCmd> ( ZypperCommand::INSTALL_NEW_RECOMMENDS_e , std::string(), { "install-new-recommends", "inr" }, InrVerifyCmd::Mode::InstallRecommends ),

      makeCmd<UpdateCmd> ( ZypperCommand::UPDATE_e , _("Update Management:"), { "update", "up"  } ),
      makeCmd<ListUpdatesCmd> ( ZypperCommand::LIST_UPDATES_e , std::string(), { "list-updates", "lu" } ),
      makeCmd<PatchCmd> ( ZypperCommand::PATCH_e , std::string(), { "patch"  } ),
      makeCmd<ListPatchesCmd> ( ZypperCommand::LIST_PATCHES_e , std::string(), { "list-patches", "lp" } ),
      makeCmd<DistUpgradeCmd> ( ZypperCommand::DIST_UPGRADE_e , std::string(), { "dist-upgrade", "dup" } ),
      makeCmd<PatchCheckCmd> ( ZypperCommand::PATCH_CHECK_e , std::string(), { "patch-check", "pchk"} ),

      makeCmd<SearchCmd> ( ZypperCommand::SEARCH_e , _("Querying:"), { "search", "se" }, SearchCmd::CmdMode::Search ),
      makeCmd<InfoCmd> ( ZypperCommand::INFO_e , std::string(), { "info", "if" } ),
      makeCmd<InfoCmd> ( ZypperCommand::RUG_PATCH_INFO_e , std::string(), { "patch-info" }, InfoCmd::Mode::RugPatchInfo ),
      makeCmd<InfoCmd> ( ZypperCommand::RUG_PATTERN_INFO_e , std::string(), { "pattern-info" }, InfoCmd::Mode::RugPatternInfo ),
      makeCmd<InfoCmd> ( ZypperCommand::RUG_PRODUCT_INFO_e , std::string(), { "product-info" }, InfoCmd::Mode::RugProductInfo ),
      makeCmd<PatchesCmd> ( ZypperCommand::PATCHES_e , std::string(), { "patches", "pch" } ),
      makeCmd<PackagesCmd> ( ZypperCommand::PACKAGES_e , std::string(), { "packages", "pa", "pkg" } ),
      makeCmd<PatternsCmd> ( ZypperCommand::PATTERNS_e , std::string(), { "patterns", "pt" } ),
      makeCmd<ProductsCmd> ( ZypperCommand::PRODUCTS_e , std::string(), { "products", "pd" } ),
      makeCmd<WhatProvidesCmd> ( ZypperCommand::WHAT_PROVIDES_e , std::string(), { "what-provides", "wp" } ),

      makeCmd<AddLocksCmd> ( ZypperCommand::ADD_LOCK_e , _("Package Locks:"), { "addlock", "al", "lock-add", "la" } ),
      makeCmd<RemoveLocksCmd> ( ZypperCommand::REMOVE_LOCK_e , std::string(), { "removelock",  "rl", "lock-delete" , "ld" } ),
      makeCmd<ListLocksCmd> ( ZypperCommand::LIST_LOCKS_e , std::string(), { "locks", "ll", "lock-list" } ),
      makeCmd<CleanLocksCmd> ( ZypperCommand::CLEAN_LOCKS_e , std::string(), { "cleanlocks" , "cl", "lock-clean" } ),

      makeCmd<VersionCompareCmd> ( ZypperCommand::VERSION_CMP_e , _("Other Commands:"), { "versioncmp", "vcmp" } ),
      makeCmd<TargetOSCmd> ( ZypperCommand::TARGET_OS_e , std::string(), { "targetos", "tos" } ),
      makeCmd<LicensesCmd> ( ZypperCommand::LICENSES_e , std::string(), { "licenses" } ),
      makeCmd<DownloadCmd> ( ZypperCommand::DOWNLOAD_e , std::string(), { "download" } ),
      makeCmd<SourceDownloadCmd> ( ZypperCommand::SOURCE_DOWNLOAD_e , std::string(), { "source-download" } ),
      makeCmd<NeedsRebootingCmd> ( ZypperCommand::NEEDS_REBOOTING_e , std::string(), { "needs-rebooting" } ),

      //SUBCOMMAND GROUP HERE

      //all commands in this group will be hidden from help
      makeCmd<PSCommand> ( ZypperCommand::PS_e , "HIDDEN", { "ps" } ),
      makeCmd<SearchCmd> ( ZypperCommand::RUG_PATCH_SEARCH_e , std::string(), { "patch-search", "pse" }, SearchCmd::CmdMode::RugPatchSearch ),
      makeCmd<ConfigTestCmd> ( ZypperCommand::CONFIGTEST_e , std::string(), { "configtest" } ),
      makeCmd<RupPingCmd> ( ZypperCommand::RUG_PING_e , std::string(), { "ping" } ),
      makeCmd<ShellQuitCmd> ( ZypperCommand::SHELL_QUIT_e , std::string(), { "quit", "exit", "\004" } ),
      makeCmd<MooCmd> ( ZypperCommand::MOO_e , std::string(), { "moo" } ),
      std::make_tuple ( ZypperCommand::NONE_e, std::string(), std::vector<std::string>{ "none", ""}, ZypperCommand::CmdFactory( voidCmd ) )
    };

    return commands;
  }

  static NamedValue<ZypperCommand::Command> & cmdTable()
  {
    static NamedValue<ZypperCommand::Command> _table;
    if ( _table.empty() )
    {
#define _t(C) _table( ZypperCommand::C )
      //_t( NONE_e )		| "NONE"		| "none" | "";
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

      //_t( INSTALL_e )		| "install"		| "in";
      //_t( REMOVE_e )		| "remove"		| "rm";
      //_t( SRC_INSTALL_e )	| "source-install"	| "si";
      //_t( VERIFY_e )		| "verify"		| "ve";
      //_t( INSTALL_NEW_RECOMMENDS_e )| "install-new-recommends" | "inr";

      //_t( UPDATE_e )		| "update"		| "up";
      //_t( LIST_UPDATES_e )	| "list-updates"	| "lu";
      //_t( PATCH_e )		| "patch";
      //_t( LIST_PATCHES_e )	| "list-patches"	| "lp";
      //_t( PATCH_CHECK_e )	| "patch-check"		| "pchk";
      //_t( DIST_UPGRADE_e )	| "dist-upgrade"	| "dup";

      //_t( SEARCH_e )		| "search"		| "se";
      //_t( INFO_e )		| "info"		| "if";
      //_t( PACKAGES_e )		| "packages"		| "pa" | "pkg";
      //_t( PATCHES_e )		| "patches"		| "pch";
      //_t( PATTERNS_e )		| "patterns"		| "pt";
      //_t( PRODUCTS_e )		| "products"		| "pd";

      //_t( WHAT_PROVIDES_e )	| "what-provides"	| "wp";
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
      //_t( DOWNLOAD_e )		| "download";
      //_t( SOURCE_DOWNLOAD_e )	| "source-download";

      //_t( HELP_e )		| "help"		| "?";
      //_t( SHELL_e )		| "shell"		| "sh";
      //_t( SHELL_QUIT_e )	| "quit"		| "exit" | "\004";
      //_t( MOO_e )		| "moo";

      //_t( CONFIGTEST_e)		|  "configtest";

      //_t( RUG_PATCH_INFO_e )	| "patch-info";
      //_t( RUG_PATTERN_INFO_e )	| "pattern-info";
      //_t( RUG_PRODUCT_INFO_e )	| "product-info";
      //_t( RUG_PATCH_SEARCH_e )	| "patch-search" | "pse";
      //_t( RUG_PING_e )		| "ping";
#undef _t

      // patch the table to contain all new style commands
      for ( const auto &cmd : newStyleCommands() ) {
        auto entry = _table( std::get< ZypperCommand::CmdDescField::Id >( cmd ) );
        for ( const std::string &alias : std::get< ZypperCommand::CmdDescField::Alias >( cmd ) )
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
  if ( !_newStyleCmdObj ) {
    //set the command object if the passed enum represents a new style cmd
    auto &newCmds = newStyleCommands();

    auto predicate = [ this ]( const CmdDesc &desc ) {
      return _command == std::get< CmdDescField::Id >( desc );
    };

    auto i = std::find_if( newCmds.begin(), newCmds.end(), predicate );
    if ( i != newCmds.end() )
      _newStyleCmdObj = std::get< CmdDescField::Factory >(*i)();
  }
  return _newStyleCmdObj;
}

const std::vector<ZypperCommand::CmdDesc> &ZypperCommand::allCommands()
{
  return newStyleCommands();
}
