/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_UTILS_H
#define ZMART_UTILS_H

#include <string>
#include "zypp/Url.h"
#include "zypp/ResObject.h"

std::string read_line_from_file( const zypp::Pathname &file );
void write_line_to_file( const zypp::Pathname &file, const std::string &line );

#endif

