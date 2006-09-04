/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_MISC_H
#define ZMART_MISC_H

#include <string>
#include "zypp/Url.h"

void mark_package_for_install( const std::string &name );
void show_summary();
std::string calculate_token();
void load_target();
void load_sources();
void resolve();
void show_pool();
void usage(int argc, char **argv);

#endif

