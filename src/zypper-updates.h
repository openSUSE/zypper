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
#include "zypp/Edition.h"

#define TOKEN_FILE "/var/lib/zypp/cache/updates_token"
#define XML_FILE_VERSION "/var/lib/zypp/cache/updates_xml_version"
#define RESULT_FILE "/var/lib/zypp/cache/updates_result.xml"

struct Error
{
  Error( const std::string &desc )
  : description(desc)
  {}
  std::string description;
};

extern std::list<Error> errors;

std::string read_old_token();
void save_token( const std::string &token );
zypp::Edition read_old_version();
void save_version( const zypp::Edition &edition );
void render_error( const zypp::Edition &version, std::ostream &out );
void render_result(  const zypp::Edition &version, std::ostream &out, const zypp::ResPool &pool);

#endif

