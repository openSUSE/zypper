/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <string>
#include "zypp/target/rpm/librpmDb.h"

void write_packages_to_db (const std::string & db_file, zypp::target::rpm::librpmDb::db_const_iterator iter);
