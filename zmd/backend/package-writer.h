/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <string>
#include "zypp/ResStore.h"

void write_packages_to_db (const std::string & db_file, const zypp::ResStore & resolvables);
