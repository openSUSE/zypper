/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include <string>
#include <list>
#include "zypp/ResStore.h"

typedef std::list<const Resolvable::constPtr> ResolvableList;

void write_store_to_db (const std::string & db_file, const zypp::ResStore & resolvables);
void write_resolvables_to_db (const std::string & db_file, const ResolvableList & resolvables);

