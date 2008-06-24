#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include "zypp/PoolItem.h"
#include "zypp/ResKind.h"
#include "zypp/ui/Selectable.h"

#include "Zypper.h"

void printInfo(const Zypper & zypper, const zypp::ResKind & kind);

void printPkgInfo(const Zypper & zypper, const zypp::ui::Selectable & s);

void printPatchInfo(const Zypper & zypper, const zypp::ui::Selectable & s);

void printPatternInfo(const Zypper & zypper, const zypp::ui::Selectable & s);

void printProductInfo(const Zypper & zypper, const zypp::ui::Selectable & s);

#endif /*ZYPPERINFO_H_*/
