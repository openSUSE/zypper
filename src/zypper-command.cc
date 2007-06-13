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
const ZypperCommand ZypperCommand::LIST_REPOS(ZypperCommand::LIST_REPOS_e);
const ZypperCommand ZypperCommand::REFRESH(ZypperCommand::REFRESH_e);
const ZypperCommand ZypperCommand::SEARCH(ZypperCommand::SEARCH_e);
const ZypperCommand ZypperCommand::INSTALL(ZypperCommand::INSTALL_e);
const ZypperCommand ZypperCommand::REMOVE(ZypperCommand::REMOVE_e);
const ZypperCommand ZypperCommand::UPDATE(ZypperCommand::UPDATE_e);

ZypperCommand::ZypperCommand(const std::string & strval_r)
  : _command(parse(strval_r))
{}

ZypperCommand::Command ZypperCommand::parse(const std::string & strval_r)
{
  if (_table.empty())
  {
    // initialize it
    _table["addrepo"] = _table["service-add"] = _table["sa"] = ZypperCommand::ADD_REPO_e;
    _table["rmrepo"] = _table["service-delete"] = _table["sd"] = ZypperCommand::REMOVE_REPO_e;
    _table["listrepos"] = _table["service-list"] = _table["sl"] = ZypperCommand::LIST_REPOS_e;
    _table["refresh"] = _table["ref"] = ZypperCommand::REFRESH_e;
    _table["search"] = _table["se"] = ZypperCommand::SEARCH_e;
    _table["install"] = _table["in"] = ZypperCommand::INSTALL_e;
    _table["remove"] = _table["rm"] = ZypperCommand::REMOVE_e;
    _table["update"] = _table["up"] = ZypperCommand::UPDATE_e;
    _table["NONE"] = _table["none"] = ZypperCommand::NONE_e;
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
