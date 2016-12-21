#ifndef ZYPPERLOCKS_H_
#define ZYPPERLOCKS_H_

#include "Zypper.h"

/** locks specific options */
struct ListLocksOptions : public Options
{
  ListLocksOptions()
    : Options( ZypperCommand::LIST_LOCKS )
    , _withMatches( false )
    , _withSolvables( false )
  {}
  bool _withMatches;
  bool _withSolvables;
};

void list_locks(Zypper & zypper);
void add_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds);
void remove_locks(Zypper & zypper, const Zypper::ArgList & args, const ResKindSet & kinds);

#endif /*ZYPPERLOCKS_H_*/
