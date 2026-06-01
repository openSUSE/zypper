/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_TRANSACTIONALWRAPPER_H_INCLUDED
#define ZYPPER_COMMANDS_TRANSACTIONALWRAPPER_H_INCLUDED

#include <string>

class TransactionalWrapper
{
public:
  int run( std::string &err_r ) const;
};

#endif
