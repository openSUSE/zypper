/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPERKEYS_H_
#define ZYPPERKEYS_H_

#include <zypp/Pathname.h>
#include <vector>
#include <string>

class Zypper;

void listTrustedKeys ( Zypper &zypper, const std::vector<std::string> &keysToList = {} , bool details = false );

void importKey ( Zypper &zypper, const std::string &url);
void removeKey ( Zypper &zypper, const std::string &searchStr, bool removeAllMatches );

#endif
