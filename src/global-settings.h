/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef GLOBAL_SETTINGS_H_INCLUDED
#define GLOBAL_SETTINGS_H_INCLUDED

/**
 * Spefifies if zypper should operate in dry-run mode
 */
struct DryRun
{
private:
  DryRun();

public:
  static const DryRun &instance ();
  static DryRun &instanceNoConst ();

  static inline bool isEnabled () {
    return instance()._enabled;
  }

  void reset();

  bool _enabled = false;

};

#endif
