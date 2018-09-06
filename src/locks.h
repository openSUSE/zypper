#ifndef ZYPPERLOCKS_H_
#define ZYPPERLOCKS_H_

#include "Zypper.h"

void remove_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds);

#endif /*ZYPPERLOCKS_H_*/
