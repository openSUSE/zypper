/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <zypp/base/LogTools.h>
#include <zypp/ui/Selectable.h>
#include <zypp/ResPool.h>

#include "Zypper.h"

#include "search-packages-hinthack.h"

using namespace zypp;

void SLE15_SearchPackagesHintHack( Zypper & zypper )
{
  if ( ! zypper.config().do_ttyout )
    return;

  Out & out( zypper.out() );
  if ( !out.typeNORMAL() || out.verbosity() < Out::NORMAL )
    return;

  // Don't hint to a subcommand if --root is used (subcommands do not support global opts)
  if ( zypper.globalOpts().changedRoot )
    return;

  ui::Selectable::Ptr plg;
  if ( ResPool::instance().empty() )
  {
    // No pool - maybe 'zypper help search': Hint if plugin script is installed
    if ( !PathInfo( "/usr/lib/zypper/commands/zypper-search-packages" ).isFile() )
      return;
  }
  else
  {
    // Hint if package is in pool
    plg = ui::Selectable::get( ResKind::package, "zypper-search-packages-plugin" );
    if ( !plg )
      return;
  }

  // So write out the hint....
  str::Str msg;
  // translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
  msg << str::Format(_("For an extended search including not yet activated remote resources please use '%1%'.")) % "zypper search-packages";
  if ( plg && plg->installedEmpty() )
    // translator: %1% denotes a zypper command to execute. Like 'zypper search-packages'.
    msg << ' ' << str::Format(_("The package providing this subcommand is currently not installed. You can install it by calling '%1%'.")) % "zypper in zypper-search-packages-plugin";

  out.notePar( msg );
}
