/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef LOCALES_H
#define LOCALES_H

#include <vector>
#include <string>
#include <map>

class Zypper;

void listLocales( Zypper & zypper, const std::vector<std::string> &localeArgs, bool showAll );

void localePackages( Zypper & zypper, const std::vector<std::string> &localeArgs, bool showAll );

void addLocales( Zypper & zypper_r, const std::vector<std::string> &localeArgs_r, bool packages, std::map<std::string, bool> *result = nullptr );

void removeLocales( Zypper & zypper, const std::vector<std::string> &localeArgs, bool packages, std::map<std::string, bool> *result = nullptr );


#endif
