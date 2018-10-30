/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED
#define ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED

class Zypper;

/** Hack for bsc#1089994: Hinting to the zypper-search-packages-plugin subcommand. */
void SLE15_SearchPackagesHintHack( Zypper & zypper );

#endif // ZYPPER_COMMANDS_SEARCH_SEARCH_PACKAGES_HINTHACK_H_INCLUDED
