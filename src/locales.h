/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef LOCALES_H
#define LOCALES_H


#include "Zypper.h"

extern ZYpp::Ptr God;

void listLocales( Zypper & zypper, std::vector<std::string> localeArgs, bool showAll );

void localePackages( Zypper & zypper, std::vector<std::string> localeArgs, bool showAll );

std::map<std::string, bool> addLocales( Zypper & zypper, std::vector<std::string> localeArgs );

void addLocalePackages( Zypper & zypper, std::vector<std::string> localeArgs );

std::map<std::string, bool> removeLocales( Zypper & zypper, std::vector<std::string> localeArgs );

void removeLocalePackages( Zypper & zypper, std::vector<std::string> localeArgs );


#endif
