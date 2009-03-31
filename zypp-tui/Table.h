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
  Ascii         = 0,           ///< | - +
  Light,
  Heavy,
  Double,
  Light3,
  Heavy3,
  LightHeavy,
  LightDouble,
  HeavyLight,
  DoubleLight,
  none,
  _End,			       ///< sentinel
};

class TableRow {
public:
  //! Constructor. Reserve place for c columns.
  TableRow (unsigned c = 0) {
    _columns.reserve (c);
  }

  void add (const string& s);

  // return number of columns
  unsigned int cols( void ) const;

  //! tab separated output
  void dumbDumpTo (ostream &stream) const;
  //! output with field widths
  void dumpTo (ostream &stream, const vector<unsigned>& widths,
	       TableStyle st, unsigned margin) const;

  typedef vector<string> container;

  // BinaryPredicate
  struct Less {
    unsigned _by_column;
    Less (unsigned by_column): _by_column (by_column) {}

    bool operator ()(const TableRow& a, const TableRow& b) const {
      return a._columns[_by_column] < b._columns[_by_column];
    }
  };

private:
  container _columns;
  friend class Table;
};

class TableHeader : public TableRow {
public:
  //! Constructor. Reserve place for c columns.
  TableHeader (unsigned c = 0): TableRow (c) {}
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
  void setHeader (const TableHeader& tr);
  void dumpTo (ostream& stream) const;
  bool empty () const { return _rows.empty(); }
  typedef list<TableRow> container;

  void style (TableStyle st);
  void sort (unsigned by_column);	// columns start with 0...
  void allowAbbrev(unsigned column);
  void margin(unsigned margin);

  Table ();

private:
  void dumpRule (ostream &stream) const;
  void updateColWidths (const TableRow& tr);

  bool _has_header;
  TableHeader _header;
  container _rows;
  //! maximum column index seen in this table
  unsigned _max_col;
  //! maximum width of respective columns
  mutable vector<unsigned> _max_width;
  //! table width (columns)
  int _width;
  //! table drawing style
  TableStyle _style;
  //! console screen width as retrieved by readline
  int _screen_width;
  //! whether to abbreviate the column if needed
  vector<bool> _abbrev_col;

  unsigned _margin;
};

inline
Table& operator << (Table& table, const TableRow& tr) {
  table.add (tr);
  return table;
}

inline
Table& operator << (Table& table, const TableHeader& tr) {
  table.setHeader (tr);
  return table;
}

inline
ostream& operator << (ostream& stream, const Table& table) {
  table.dumpTo (stream);
  return stream;
}
// Local Variables:
// c-basic-offset: 2
// End:
#endif
