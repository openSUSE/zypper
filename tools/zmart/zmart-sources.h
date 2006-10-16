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

//! calls init_system_sources if not disabled by user (or non-root)
void cond_init_system_sources();
void init_system_sources();
void include_source_by_url( const zypp::Url &url );
bool parse_repo_file (const std::string& file, std::string& url, std::string& alias);
void add_source_by_url( const zypp::Url &url, std::string alias );
void remove_source( const std::string& anystring );
void rename_source( const std::string& anystring, const std::string& newalias );
void list_system_sources();

#endif

