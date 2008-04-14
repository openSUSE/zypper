#ifndef ZYPPERLOCKS_H_
#define ZYPPERLOCKS_H_

#include "zypper.h"

void list_locks(Zypper & zypper);
void add_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds);
void remove_locks(Zypper & zypper, const Zypper::ArgList & args);

#endif /*ZYPPERLOCKS_H_*/
