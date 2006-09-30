/*-----------------------------------------------------------*- c++ -*-\
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
#include "zypp/ResObject.h"

bool readBoolAnswer();
zypp::ResObject::Kind string_to_kind (const std::string &skind);
void mark_for_install( const zypp::ResObject::Kind &kind,
		       const std::string &name );
void mark_for_uninstall( const zypp::ResObject::Kind &kind,
			 const std::string &name );
void show_summary();
std::string calculate_token();
//! load all resolvables that the user wants
void cond_load_resolvables ();
void load_target();
void load_sources();
void establish ();
void resolve();
void show_pool();
void usage(int argc, char **argv);

#endif

