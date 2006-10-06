/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_UPDATES_H
#define ZMART_UPDATES_H

#include <string>
#include "zypp/Url.h"
#include "zypp/ResObject.h"

#define TOKEN_FILE "/var/lib/zypp/cache/updates_token"
#define RESULT_FILE "/var/lib/zypp/cache/updates_result.xml"

std::string read_old_token();
void save_token( const std::string &token );
void render_error(  std::ostream &out, const std::string &reason );
void render_unchanged(  std::ostream &out, const std::string &token );
void render_result( std::ostream &out, const zypp::ResPool &pool);

#endif

