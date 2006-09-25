/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_SOURCES_H
#define ZMART_SOURCES_H

#include "zypp/Url.h"

void init_system_sources();
void include_source_by_url( const zypp::Url &url );
void add_source_by_url( const zypp::Url &url, std::string alias );
void remove_source( const std::string anystring );
void list_system_sources();

#endif

