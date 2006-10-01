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

//! table drawing style
enum TableStyle {
  Ascii,			///< | - +
  Fancy,			///< unspecified unicode fancy style
  Heavy,
  Light,
  _End,				///< sentinel
};

class TableRow {
public:
  void add (const string& s);

  //! tab separated output
  void dumbDumpTo (ostream &stream) const;
  //! output with field widths
  void dumpTo (ostream &stream, const vector<unsigned>& widths,
	       TableStyle st) const;

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
  static TableStyle defaultStyle;

  void add (const TableRow& tr);
  void dumpTo (ostream& stream) const;
  typedef list<TableRow> container;

  void style (TableStyle st);
  void hasHeader (bool b) { _has_header = b; }

  Table () :
    _has_header (false),
    _max_col (0),
    _style (defaultStyle)
    {}
private:
  void dumpRule (ostream &stream) const;

  bool _has_header;
  container _rows;
  //! maximum column index seen in this table
  unsigned _max_col;
  //! maximum width of respective columns
  vector<unsigned> _max_width;
  //! table drawing style
  TableStyle _style;
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
