/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "global-settings.h"

void GlobalSettings::reset()
{
  DryRunSettings::reset();
  InitRepoSettings::reset();
}

void DryRunSettingsData::reset()
{
  _enabled = false;
}

void InitRepoSettingsData::reset()
{
  _repoFilter.clear();
}
