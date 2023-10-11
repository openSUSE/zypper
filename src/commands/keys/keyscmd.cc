/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "keyscmd.h"
#include "Zypper.h"
#include "keys.h"
#include "commands/commonflags.h"

KeysCmd::KeysCmd( std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate the command 'name (abbreviations)' or '-option' names
    _("keys [OPTIONS] [KEYID] [KEYFILE] ..."),
    // translators: command summary
    _("List all keys."),
    // translators: command description
    _("List all trusted keys or show detailed information about those specified as arguments, supports also keyfiles as argument."),
    InitTarget )
{
  init();
}

void KeysCmd::init()
{
  _showDetails = false;
}

zypp::ZyppFlags::CommandGroup KeysCmd::cmdOptions() const
{
  auto &that = *const_cast<KeysCmd *>(this);
  return {{
    CommonFlags::detailsFlag( that._showDetails, '\0', _("Show more details.") )
  }};
}

void KeysCmd::doReset()
{
  init();
}

int KeysCmd::execute( Zypper &zypper, const std::vector<std::string> & )
{
  listTrustedKeys( zypper, positionalArguments(), _showDetails );
  return zypper.exitCode();
}
