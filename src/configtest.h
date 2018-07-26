/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_CONFIGTEST_H
#define ZYPPER_CONFIGTEST_H

class Zypper;

///////////////////////////////////////////////////////////////////
/// \class ConfigtestOptions
/// \brief \ref Configtest specific options
///////////////////////////////////////////////////////////////////
struct ConfigtestOptions // A FAKED command which can't use class Options
{};
///////////////////////////////////////////////////////////////////

/** Configtest debug command
 * \returns zypper.exitCode
 */
int configtest( Zypper & zypper_r );

#endif // ZYPPER_CONFIGTEST_H
