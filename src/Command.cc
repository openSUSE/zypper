/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <map>

#include "zypp/base/Exception.h"
#include "zypp/base/String.h"

#include "main.h"
#include "Command.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)


static std::map<std::string,ZypperCommand::Command> _table;

const ZypperCommand ZypperCommand::ADD_SERVICE(ZypperCommand::ADD_SERVICE_e);
const ZypperCommand ZypperCommand::REMOVE_SERVICE(ZypperCommand::REMOVE_SERVICE_e);
const ZypperCommand ZypperCommand::MODIFY_SERVICE(ZypperCommand::MODIFY_SERVICE_e);
const ZypperCommand ZypperCommand::LIST_SERVICES(ZypperCommand::LIST_SERVICES_e);
const ZypperCommand ZypperCommand::REFRESH_SERVICES(ZypperCommand::REFRESH_SERVICES_e);

const ZypperCommand ZypperCommand::ADD_REPO(ZypperCommand::ADD_REPO_e);
const ZypperCommand ZypperCommand::REMOVE_REPO(ZypperCommand::REMOVE_REPO_e);
const ZypperCommand ZypperCommand::RENAME_REPO(ZypperCommand::RENAME_REPO_e);
const ZypperCommand ZypperCommand::MODIFY_REPO(ZypperCommand::MODIFY_REPO_e);
const ZypperCommand ZypperCommand::LIST_REPOS(ZypperCommand::LIST_REPOS_e);
const ZypperCommand ZypperCommand::REFRESH(ZypperCommand::REFRESH_e);
const ZypperCommand ZypperCommand::CLEAN(ZypperCommand::CLEAN_e);

const ZypperCommand ZypperCommand::INSTALL(ZypperCommand::INSTALL_e);
const ZypperCommand ZypperCommand::REMOVE(ZypperCommand::REMOVE_e);
const ZypperCommand ZypperCommand::SRC_INSTALL(ZypperCommand::SRC_INSTALL_e);
const ZypperCommand ZypperCommand::VERIFY(ZypperCommand::VERIFY_e);
const ZypperCommand ZypperCommand::INSTALL_NEW_RECOMMENDS(ZypperCommand::INSTALL_NEW_RECOMMENDS_e);

const ZypperCommand ZypperCommand::UPDATE(ZypperCommand::UPDATE_e);
const ZypperCommand ZypperCommand::LIST_UPDATES(ZypperCommand::LIST_UPDATES_e);
const ZypperCommand ZypperCommand::PATCH(ZypperCommand::PATCH_e);
const ZypperCommand ZypperCommand::LIST_PATCHES(ZypperCommand::LIST_PATCHES_e);
const ZypperCommand ZypperCommand::PATCH_CHECK(ZypperCommand::PATCH_CHECK_e);
const ZypperCommand ZypperCommand::DIST_UPGRADE(ZypperCommand::DIST_UPGRADE_e);

const ZypperCommand ZypperCommand::SEARCH(ZypperCommand::SEARCH_e);
const ZypperCommand ZypperCommand::INFO(ZypperCommand::INFO_e);
const ZypperCommand ZypperCommand::PACKAGES(ZypperCommand::PACKAGES_e);
const ZypperCommand ZypperCommand::PATCHES(ZypperCommand::PATCHES_e);
const ZypperCommand ZypperCommand::PATTERNS(ZypperCommand::PATTERNS_e);
const ZypperCommand ZypperCommand::PRODUCTS(ZypperCommand::PRODUCTS_e);
const ZypperCommand ZypperCommand::WHAT_PROVIDES(ZypperCommand::WHAT_PROVIDES_e);
//const ZypperCommand ZypperCommand::WHAT_REQUIRES(ZypperCommand::WHAT_REQUIRES_e);
//const ZypperCommand ZypperCommand::WHAT_CONFLICTS(ZypperCommand::WHAT_CONFLICTS_e);

const ZypperCommand ZypperCommand::ADD_LOCK(ZypperCommand::ADD_LOCK_e);
const ZypperCommand ZypperCommand::REMOVE_LOCK(ZypperCommand::REMOVE_LOCK_e);
const ZypperCommand ZypperCommand::LIST_LOCKS(ZypperCommand::LIST_LOCKS_e);
const ZypperCommand ZypperCommand::CLEAN_LOCKS(ZypperCommand::CLEAN_LOCKS_e);

const ZypperCommand ZypperCommand::TARGET_OS(ZypperCommand::TARGET_OS_e);
const ZypperCommand ZypperCommand::VERSION_CMP(ZypperCommand::VERSION_CMP_e);
const ZypperCommand ZypperCommand::LICENSES(ZypperCommand::LICENSES_e);
const ZypperCommand ZypperCommand::PS(ZypperCommand::PS_e);

const ZypperCommand ZypperCommand::HELP(ZypperCommand::HELP_e);
const ZypperCommand ZypperCommand::SHELL(ZypperCommand::SHELL_e);
const ZypperCommand ZypperCommand::SHELL_QUIT(ZypperCommand::SHELL_QUIT_e);
const ZypperCommand ZypperCommand::NONE(ZypperCommand::NONE_e);
const ZypperCommand ZypperCommand::MOO(ZypperCommand::MOO_e);


const ZypperCommand ZypperCommand::RUG_PATCH_INFO(ZypperCommand::RUG_PATCH_INFO_e);
const ZypperCommand ZypperCommand::RUG_PATTERN_INFO(ZypperCommand::RUG_PATTERN_INFO_e);
const ZypperCommand ZypperCommand::RUG_PRODUCT_INFO(ZypperCommand::RUG_PRODUCT_INFO_e);
const ZypperCommand ZypperCommand::RUG_SERVICE_TYPES(ZypperCommand::RUG_SERVICE_TYPES_e);
const ZypperCommand ZypperCommand::RUG_LIST_RESOLVABLES(ZypperCommand::RUG_LIST_RESOLVABLES_e);
const ZypperCommand ZypperCommand::RUG_MOUNT(ZypperCommand::RUG_MOUNT_e);
//const ZypperCommand ZypperCommand::RUG_INFO_PROVIDES(ZypperCommand::RUG_INFO_PROVIDES_e);
//const ZypperCommand ZypperCommand::RUG_INFO_CONFLICTS(ZypperCommand::RUG_INFO_CONFLICTS_e);
//const ZypperCommand ZypperCommand::RUG_INFO_OBSOLETES(ZypperCommand::RUG_INFO_OBSOLETES_e);
//const ZypperCommand ZypperCommand::RUG_INFO_REQUIREMENTS(ZypperCommand::RUG_INFO_REQUIREMENTS_e);
const ZypperCommand ZypperCommand::RUG_PATCH_SEARCH(ZypperCommand::RUG_PATCH_SEARCH_e);
const ZypperCommand ZypperCommand::RUG_PING(ZypperCommand::RUG_PING_e);


ZypperCommand::ZypperCommand(const std::string & strval_r)
  : _command(parse(strval_r))
{}

ZypperCommand::Command ZypperCommand::parse(const std::string & strval_r)
{
  if (_table.empty())
  {
    // initialize it
    _table["addservice"] = _table["as"] = _table["service-add"] = _table["sa"] = ZypperCommand::ADD_SERVICE_e;
    _table["removeservice"] = _table["rs"] = _table["service-delete"] = _table["sd"] = ZypperCommand::REMOVE_SERVICE_e;
    _table["modifyservice"] = _table["ms"] = ZypperCommand::MODIFY_SERVICE_e;
    _table["services"] = _table["ls"] = _table["service-list"] = _table["sl"] = ZypperCommand::LIST_SERVICES_e;
    _table["refresh-services"] = _table["refs"] = ZypperCommand::REFRESH_SERVICES_e;

    _table["addrepo"] = _table["ar"] = ZypperCommand::ADD_REPO_e;
    _table["removerepo"] = _table["rr"] = ZypperCommand::REMOVE_REPO_e;
    _table["renamerepo"]= _table["nr"] = ZypperCommand::RENAME_REPO_e;
    _table["modifyrepo"]= _table["mr"] = ZypperCommand::MODIFY_REPO_e;
    _table["repos"] = _table["lr"] = _table["catalogs"] = _table["ca"] = ZypperCommand::LIST_REPOS_e;
    _table["refresh"] = _table["ref"] = ZypperCommand::REFRESH_e;
    _table["clean"] = _table["clean-cache"] = _table["cc"] = _table["you-clean-cache"] = _table["yc"] = ZypperCommand::CLEAN_e;

    _table["install"] = _table["in"] = ZypperCommand::INSTALL_e;
    _table["remove"] = _table["rm"] = ZypperCommand::REMOVE_e;
    _table["source-install"] = _table["si"] = ZypperCommand::SRC_INSTALL_e;
    _table["verify"] = _table["ve"] = ZypperCommand::VERIFY_e;
    _table["install-new-recommends"] = _table["inr"] = ZypperCommand::INSTALL_NEW_RECOMMENDS_e;

    _table["update"] = _table["up"] = ZypperCommand::UPDATE_e;
    _table["list-updates"] = _table["lu"] = ZypperCommand::LIST_UPDATES_e;
    _table["patch"] = ZypperCommand::PATCH_e;
    _table["list-patches"] = _table["lp"] = ZypperCommand::LIST_PATCHES_e;
    _table["patch-check"] = _table["pchk"] = ZypperCommand::PATCH_CHECK_e;
    _table["dist-upgrade"] = _table["dup"] = ZypperCommand::DIST_UPGRADE_e;

    _table["search"] = _table["se"] = ZypperCommand::SEARCH_e;
    _table["info"] = _table["if"] = ZypperCommand::INFO_e;
    _table["packages"] = _table["pa"] = _table["pkg"] = ZypperCommand::PACKAGES_e;
    _table["patches"] = _table["pch"] = ZypperCommand::PATCHES_e;
    _table["patterns"] = _table["pt"] = ZypperCommand::PATTERNS_e;
    _table["products"] = _table["pd"] = ZypperCommand::PRODUCTS_e;
    _table["what-provides"] = _table["wp"] = ZypperCommand::WHAT_PROVIDES_e;
    //_table["what-requires"] = _table["wr"] = ZypperCommand::WHAT_REQUIRES_e;
    //_table["what-conflicts"] = _table["wc"] = ZypperCommand::WHAT_CONFLICTS_e;

    _table["addlock"] = _table["al"] = _table["lock-add"] = _table["la"] = ZypperCommand::ADD_LOCK_e;
    _table["removelock"] = _table["rl"] = _table["lock-delete"] = _table["ld"] = ZypperCommand::REMOVE_LOCK_e;
    _table["locks"] = _table["ll"] = _table["lock-list"] = ZypperCommand::LIST_LOCKS_e;
    _table["cleanlocks"] = _table["cl"] = _table["lock-clean"] = ZypperCommand::CLEAN_LOCKS_e;

    _table["targetos"] = _table["tos"] = ZypperCommand::TARGET_OS_e;
    _table["versioncmp"] = _table["vcmp"] = ZypperCommand::VERSION_CMP_e;
    _table["licenses"] = ZypperCommand::LICENSES_e;
    _table["ps"] = ZypperCommand::PS_e;

    _table["help"] = _table["?"] = ZypperCommand::HELP_e;
    _table["shell"] = _table["sh"] = ZypperCommand::SHELL_e;
    _table["quit"] = _table["exit"] = _table["\004"] = ZypperCommand::SHELL_QUIT_e;
    _table["NONE"] = _table["none"] = _table[""] = ZypperCommand::NONE_e;
    _table["moo"] = ZypperCommand::MOO_e;


    // rug commands doable with zypper commands
    _table["patch-info"] = ZypperCommand::RUG_PATCH_INFO_e;
    _table["pattern-info"] = ZypperCommand::RUG_PATTERN_INFO_e;
    _table["product-info"] = ZypperCommand::RUG_PRODUCT_INFO_e;
    _table["service-types"] = _table["st"] = ZypperCommand::RUG_SERVICE_TYPES_e;
    _table["list-resolvables"] = /*_table["lr"] CONFLICT with repos =*/ ZypperCommand::RUG_LIST_RESOLVABLES_e;
    _table["mount"] = ZypperCommand::RUG_MOUNT_e;
    //_table["info-provides"] = _table["ip"] = ZypperCommand::RUG_INFO_PROVIDES_e;
    //_table["info-requirements"] = _table["ir"] = ZypperCommand::RUG_INFO_REQUIREMENTS_e;
    //_table["info-conflicts"] = _table["ic"] = ZypperCommand::RUG_INFO_CONFLICTS_e;
    //_table["info-obsoletes"] = _table["io"] = ZypperCommand::RUG_INFO_OBSOLETES_e;
    _table["patch-search"] = _table["pse"] = ZypperCommand::RUG_PATCH_SEARCH_e;
    _table["ping"] = ZypperCommand::RUG_PING_e;
  }

  std::map<std::string,ZypperCommand::Command>::const_iterator it
    = _table.find(strval_r);
  if (it == _table.end())
  {
    std::string message =
      zypp::str::form(_("Unknown command '%s'"), strval_r.c_str());
    ZYPP_THROW(zypp::Exception(message));
  }
  return it->second;
}
