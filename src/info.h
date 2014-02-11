/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include <zypp/PoolItem.h>
#include <zypp/ResKind.h>
#include <zypp/ui/Selectable.h>

#include "Zypper.h"

void printInfo(Zypper & zypper, const zypp::ResKind & kind);

#endif /*ZYPPERINFO_H_*/
