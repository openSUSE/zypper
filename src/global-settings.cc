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
  SolverSettings::reset();
  LicenseAgreementPolicy::reset();
  DupSettings::reset();
  FileConflictPolicy::reset();
}
