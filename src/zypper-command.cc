#include <map>

#include <boost/format.hpp>

#include <zypp/Locale.h>
#include <zypp/base/Exception.h>

#include "zypper-command.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)


static std::map<std::string,ZypperCommand::Command> _table;

const ZypperCommand ZypperCommand::ADD_REPO(ZypperCommand::ADD_REPO_e);
const ZypperCommand ZypperCommand::REMOVE_REPO(ZypperCommand::REMOVE_REPO_e);
const ZypperCommand ZypperCommand::RENAME_REPO(ZypperCommand::RENAME_REPO_e);
const ZypperCommand ZypperCommand::MODIFY_REPO(ZypperCommand::MODIFY_REPO_e);
const ZypperCommand ZypperCommand::LIST_REPOS(ZypperCommand::LIST_REPOS_e);
const ZypperCommand ZypperCommand::REFRESH(ZypperCommand::REFRESH_e);

const ZypperCommand ZypperCommand::INSTALL(ZypperCommand::INSTALL_e);
const ZypperCommand ZypperCommand::REMOVE(ZypperCommand::REMOVE_e);
const ZypperCommand ZypperCommand::UPDATE(ZypperCommand::UPDATE_e);
const ZypperCommand ZypperCommand::SRC_INSTALL(ZypperCommand::SRC_INSTALL_e);

const ZypperCommand ZypperCommand::SEARCH(ZypperCommand::SEARCH_e);
const ZypperCommand ZypperCommand::INFO(ZypperCommand::INFO_e);
const ZypperCommand ZypperCommand::LIST_UPDATES(ZypperCommand::LIST_UPDATES_e);
const ZypperCommand ZypperCommand::PATCH_CHECK(ZypperCommand::PATCH_CHECK_e);
const ZypperCommand ZypperCommand::SHOW_PATCHES(ZypperCommand::SHOW_PATCHES_e);
const ZypperCommand ZypperCommand::XML_LIST_UPDATES_PATCHES(ZypperCommand::XML_LIST_UPDATES_PATCHES_e);

const ZypperCommand ZypperCommand::HELP(ZypperCommand::HELP_e);
const ZypperCommand ZypperCommand::SHELL(ZypperCommand::SHELL_e);
const ZypperCommand ZypperCommand::SHELL_QUIT(ZypperCommand::SHELL_QUIT_e);
const ZypperCommand ZypperCommand::NONE(ZypperCommand::NONE_e);
const ZypperCommand ZypperCommand::MOO(ZypperCommand::MOO_e);


const ZypperCommand ZypperCommand::RUG_PATCH_INFO(ZypperCommand::RUG_PATCH_INFO_e);


ZypperCommand::ZypperCommand(const std::string & strval_r)
  : _command(parse(strval_r))
{}

ZypperCommand::Command ZypperCommand::parse(const std::string & strval_r)
{
  if (_table.empty())
  {
    // initialize it
    _table["addrepo"] = _table["ar"] = _table["service-add"] = _table["sa"] = ZypperCommand::ADD_REPO_e;
    _table["removerepo"] = _table["rr"] = _table["service-delete"] = _table["sd"] = ZypperCommand::REMOVE_REPO_e;
    _table["renamerepo"]= _table["nr"] = _table["service-rename"] = _table["sr"] = ZypperCommand::RENAME_REPO_e;    
    _table["modifyrepo"]= _table["mr"] = _table["service-modify"] = _table["sm"] = ZypperCommand::MODIFY_REPO_e;    
    _table["repos"] = _table["lr"] = _table["service-list"] = _table["sl"] = ZypperCommand::LIST_REPOS_e;
    _table["refresh"] = _table["ref"] = ZypperCommand::REFRESH_e;

    _table["install"] = _table["in"] = ZypperCommand::INSTALL_e;
    _table["remove"] = _table["rm"] = ZypperCommand::REMOVE_e;
    _table["update"] = _table["up"] = ZypperCommand::UPDATE_e;
    _table["source-install"] = _table["si"] = ZypperCommand::SRC_INSTALL_e;

    _table["search"] = _table["se"] = ZypperCommand::SEARCH_e;
    _table["info"] = _table["if"] = ZypperCommand::INFO_e;
    _table["list-updates"] = _table["lu"] = ZypperCommand::LIST_UPDATES_e;
    _table["patch-check"] = _table["pchk"] = ZypperCommand::PATCH_CHECK_e;
    _table["patches"] = _table["pch"] = ZypperCommand::SHOW_PATCHES_e;
    _table["xml-updates"] = _table["xu"] = ZypperCommand::XML_LIST_UPDATES_PATCHES_e;

    _table["help"] = _table["?"] = ZypperCommand::HELP_e;
    _table["shell"] = _table["sh"] = ZypperCommand::SHELL_e;
    _table["quit"] = _table["exit"] = _table["\004"] = ZypperCommand::SHELL_QUIT_e;
    _table["NONE"] = _table["none"] = _table[""] = ZypperCommand::NONE_e;
    _table["moo"] = ZypperCommand::MOO_e;


    // rug commands doable with zypper commands
    _table["patch-info"] = ZypperCommand::RUG_PATCH_INFO_e;
  }

  std::map<std::string,ZypperCommand::Command>::const_iterator it
    = _table.find(strval_r);
  if (it == _table.end())
  {
    std::string message =
      boost::str( boost::format(_("Unknown command '%s'")) % strval_r );
    ZYPP_THROW(zypp::Exception(message));
  }
  return it->second;
}
