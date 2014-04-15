/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <map>

#include <zypp/base/NamedValue.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>

#include "main.h"
#include "Command.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

///////////////////////////////////////////////////////////////////
namespace
{
  static zypp::NamedValue<ZypperCommand::Command> & table()
  {
    static zypp::NamedValue<ZypperCommand::Command> _table;
    if ( _table.empty() )
    {
#define _T(C) _table( ZypperCommand::C )
      _T( NONE_e )		| "NONE"		| "none" | "";

      _T( ADD_SERVICE_e )	| "addservice"		| "as" | "service-add" | "sa";
      _T( REMOVE_SERVICE_e )	| "removeservice"	| "rs" | "service-delete" | "sd";
      _T( MODIFY_SERVICE_e )	| "modifyservice"	| "ms";
      _T( LIST_SERVICES_e )	| "services"		| "ls" | "service-list" | "sl";
      _T( REFRESH_SERVICES_e )	| "refresh-services"	| "refs";

      _T( ADD_REPO_e )		| "addrepo" 		| "ar";
      _T( REMOVE_REPO_e )	| "removerepo"		| "rr";
      _T( RENAME_REPO_e )	| "renamerepo"		| "nr";
      _T( MODIFY_REPO_e )	| "modifyrepo"		| "mr";
      _T( LIST_REPOS_e )	| "repos"		| "lr" | "catalogs" | "ca";
      _T( REFRESH_e )		| "refresh"		| "ref";
      _T( CLEAN_e )		| "clean"		| "cc" | "clean-cache" | "you-clean-cache" | "yc";

      _T( INSTALL_e )		| "install"		| "in";
      _T( REMOVE_e )		| "remove"		| "rm";
      _T( SRC_INSTALL_e )	| "source-install"	| "si";
      _T( VERIFY_e )		| "verify"		| "ve";
      _T( INSTALL_NEW_RECOMMENDS_e )| "install-new-recommends" | "inr";

      _T( UPDATE_e )		| "update"		| "up";
      _T( LIST_UPDATES_e )	| "list-updates"	| "lu";
      _T( PATCH_e )		| "patch";
      _T( LIST_PATCHES_e )	| "list-patches"	| "lp";
      _T( PATCH_CHECK_e )	| "patch-check"		| "pchk";
      _T( DIST_UPGRADE_e )	| "dist-upgrade"	| "dup";

      _T( SEARCH_e )		| "search"		| "se";
      _T( INFO_e )		| "info"		| "if";
      _T( PACKAGES_e )		| "packages"		| "pa" | "pkg";
      _T( PATCHES_e )		| "patches"		| "pch";
      _T( PATTERNS_e )		| "patterns"		| "pt";
      _T( PRODUCTS_e )		| "products"		| "pd";

      _T( WHAT_PROVIDES_e )	| "what-provides"	| "wp";
      //_T( WHAT_REQUIRES_e )	| "what-requires"	| "wr";
      //_T( WHAT_CONFLICTS_e )	| "what-conflicts"	| "wc";

      _T( ADD_LOCK_e )		| "addlock"		| "al" | "lock-add" | "la";
      _T( REMOVE_LOCK_e )	| "removelock"		| "rl" | "lock-delete" | "ld";
      _T( LIST_LOCKS_e )	| "locks"		| "ll" | "lock-list";
      _T( CLEAN_LOCKS_e )	| "cleanlocks"		| "cl" | "lock-clean";

      _T( TARGET_OS_e )		| "targetos"		| "tos";
      _T( VERSION_CMP_e )	| "versioncmp"		| "vcmp";
      _T( LICENSES_e )		| "licenses";
      _T( PS_e )		| "ps";
      _T( DOWNLOAD_e )		| "download";
      _T( SOURCE_DOWNLOAD_e )	| "source-download";

      _T( HELP_e )		| "help"		| "?";
      _T( SHELL_e )		| "shell"		| "sh";
      _T( SHELL_QUIT_e )	| "quit"		| "exit" | "\004";
      _T( MOO_e )		| "moo";

      _T( RUG_PATCH_INFO_e )	| "patch-info";
      _T( RUG_PATTERN_INFO_e )	| "pattern-info";
      _T( RUG_PRODUCT_INFO_e )	| "product-info";
      _T( RUG_SERVICE_TYPES_e )	| "service-types"	| "st";
      _T( RUG_LIST_RESOLVABLES_e )| "list-resolvables";	// "lr" CONFLICT with repos
      _T( RUG_MOUNT_e )		| "mount";
      //_T( RUG_INFO_PROVIDES_e )| "info-provides"	| "ip";
      //_T( RUG_INFO_CONFLICTS_e )| "info-requirements"	| "ir";
      //_T( RUG_INFO_OBSOLETES_e )| "info-conflicts"	| "ic";
      //_T( RUG_INFO_REQUIREMENTS_e )| "info-obsoletes"	| "io";
      _T( RUG_PATCH_SEARCH_e )	| "patch-search" | "pse";
      _T( RUG_PING_e )		| "ping";
#undef _T
    }
    return _table;
  }
} // namespace
///////////////////////////////////////////////////////////////////

#define DEF_ZYPPER_COMMAND(C) const ZypperCommand ZypperCommand::C( ZypperCommand::C##_e )
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
DEF_ZYPPER_COMMAND( NONE );
DEF_ZYPPER_COMMAND( MOO );

DEF_ZYPPER_COMMAND( RUG_PATCH_INFO );
DEF_ZYPPER_COMMAND( RUG_PATTERN_INFO );
DEF_ZYPPER_COMMAND( RUG_PRODUCT_INFO );
DEF_ZYPPER_COMMAND( RUG_SERVICE_TYPES );
DEF_ZYPPER_COMMAND( RUG_LIST_RESOLVABLES );
DEF_ZYPPER_COMMAND( RUG_MOUNT );
//DEF_ZYPPER_COMMAND( RUG_INFO_PROVIDES );
//DEF_ZYPPER_COMMAND( RUG_INFO_CONFLICTS );
//DEF_ZYPPER_COMMAND( RUG_INFO_OBSOLETES );
//DEF_ZYPPER_COMMAND( RUG_INFO_REQUIREMENTS );
DEF_ZYPPER_COMMAND( RUG_PATCH_SEARCH );
DEF_ZYPPER_COMMAND( RUG_PING );

#undef DEF_ZYPPER_COMMAND
///////////////////////////////////////////////////////////////////

ZypperCommand::ZypperCommand( const std::string & strval_r )
  : _command( parse(strval_r ) )
{}

ZypperCommand::Command ZypperCommand::parse( const std::string & strval_r ) const
{
  ZypperCommand::Command cmd;
  if ( ! table().getValue( strval_r, cmd ) )
  {
    ZYPP_THROW( zypp::Exception( zypp::str::form(_("Unknown command '%s'"), strval_r.c_str() ) ) );
  }
  return cmd;
}

const std::string & ZypperCommand::asString() const
{
  return table().getName( _command );
}
