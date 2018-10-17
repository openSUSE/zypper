/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "global-settings.h"

DryRun::DryRun()
{}

const DryRun &DryRun::instance()
{
  return instanceNoConst();
}

DryRun &DryRun::instanceNoConst()
{
  static DryRun me;
  return me;
}

void DryRun::reset()
{
  _enabled = false;
}
