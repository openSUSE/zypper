/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef LOCALES_H
#define LOCALES_H


#include "Zypper.h"

void listLocales( Zypper & zypper, vector<string> localeArgs, bool showAll );

void localePackages( Zypper & zypper, vector<string> localeArgs, bool showAll );

void addLocales( Zypper & zypper, vector<string> localeArgs );

void addLocalePackages( Zypper & zypper, vector<string> localeArgs );

void removeLocales( Zypper & zypper, vector<string> localeArgs );

void removeLocalePackages( Zypper & zypper, vector<string> localeArgs );


#endif
