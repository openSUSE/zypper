/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <string>
#include <list>
#include "zypp/ResStore.h"
#include "zypp/Resolvable.h"

typedef std::list<zypp::Resolvable::constPtr> ResolvableList;

void write_store_to_db (const std::string & db_file, const zypp::ResStore & resolvables, bool is_installed);
void write_resolvables_to_db (const std::string & db_file, const ResolvableList & resolvables, bool is_installed);
