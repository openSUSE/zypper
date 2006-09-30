/*-----------------------------------------------------------*- c++ -*-\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPPER_TABULATOR_H
#define ZYPPER_TABULATOR_H

#include <string>
#include <iosfwd>
#include <list>
#include <vector>
using std::string;
using std::ostream;
using std::list;
using std::vector;

class TableRow {
public:
  void add (const string& s);

  //! tab separated output
  void dumbDumpTo (ostream &stream) const;
  //! output with field widths
  void dumpTo (ostream &stream, const vector<unsigned>& widths) const;

  typedef list<string> container;
private:
  container _columns;
  friend class Table;
};

inline
TableRow& operator << (TableRow& tr, const string& s) {
  tr.add (s);
  return tr;
}

class Table {
public:
  void add (const TableRow& tr);
  void dumpTo (ostream& stream) const;

  typedef list<TableRow> container;
private:
  container _rows;
  //! maximum column index seen in this table
  unsigned _max_col;
  //! maximum width of respective columns
  vector<unsigned> _max_width;
};

inline
Table& operator << (Table& table, const TableRow& tr) {
  table.add (tr);
  return table;
}

inline
ostream& operator << (ostream& stream, const Table& table) {
  table.dumpTo (stream);
  return stream;
}

#endif
