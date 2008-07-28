#ifndef INSTALL_H_
#define INSTALL_H_

#include "zypp/PoolItem.h"

#include "Zypper.h"


void install_remove(Zypper & zypper,
                    const Zypper::ArgList & args,
                    bool install_not_remove,
                    const zypp::ResKind & kind);

#endif /*INSTALL_H_*/
