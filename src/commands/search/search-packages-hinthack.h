/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED
#define ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED

class Zypper;

///////////////////////////////////////////////////////////////////
namespace searchPackagesHintHack
{
  extern int argvCmdIdx;	///< opt parser remembered argv index of the command name
  extern int argvArgIdx;	///< opt parser remembered argv index of 1st positional arg (or argc if none)

  /** Hack for bsc#1089994: SLE15: Hinting to the \c search-packages subcommand (package \c zypper-search-packages-plugin).
   * FATE#325599: Introduced with SLE15-SP1 and forces us to even call the plugin at the end of a search command.
   */
  void callOrNotify( Zypper & zypper_r );

} // searchPackagesHintHack
///////////////////////////////////////////////////////////////////
#endif // ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED
