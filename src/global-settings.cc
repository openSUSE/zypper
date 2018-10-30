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
}

void DryRunSettingsData::reset()
{
  _enabled = false;
}

void InitRepoSettingsData::reset()
{
  _repoFilter.clear();
}

void SolverSettingsData::reset()
{
  _debugSolver        = zypp::indeterminate;
  _forceResolution    = zypp::indeterminate;
  _recommends         = zypp::indeterminate;
  _allowDowngrade     = zypp::indeterminate;
  _allowNameChange    = zypp::indeterminate;
  _allowVendorChange  = zypp::indeterminate;
  _allowArchChange    = zypp::indeterminate;
  _cleanDeps          = zypp::indeterminate;
}

void LicenseAgreementPolicyData::reset()
{
  _autoAgreeWithLicenses = false;
  _autoAgreeWithProductLicenses = false;
}
