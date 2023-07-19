/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_TABULATOR_H
#define ZYPPER_TABULATOR_H

#include <zypp-tui/Table.h>

using ztui::Table;
using ztui::TableRow;
using ztui::TableHeader;
using ztui::PropertyTable;
using ztui::asYesNo;
using ztui::SolvableCSI;
using ztui::TableLineStyle;
using ztui::ColumnIf;

namespace table {
  using ztui::table::CStyle;
  using ztui::table::Column;
}

#endif // ZYPPER_TABULATOR_H
