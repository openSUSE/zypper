#include <iostream>
#include "zypper-tabulator.h"
using namespace std;

void TableRow::add (const string& s) {
  _columns.push_back (s);
}

// 1st implementation: no width calculation, just tabs
void TableRow::dumbDumpTo (ostream &stream) const {
  bool seen_first = false;
  for (container::const_iterator i = _columns.begin (); i != _columns.end (); ++i) {
    if (seen_first)
      stream << '\t';
    seen_first = true;

    stream << *i;
  }
  stream << endl;
}

void TableRow::dumpTo (ostream &stream, const vector<unsigned>& widths) const {
  const char* separator = " | ";
  bool seen_first = false;
  container::const_iterator
    i = _columns.begin (),
    e = _columns.end ();
  
  stream.setf (ios::left, ios::adjustfield);
  for (unsigned c = 0; i != e ; ++i, ++c) {
    if (seen_first) {
      stream.width (0);
      stream << separator;
    }
    seen_first = true;

    stream.width (widths[c]);
    stream << *i;
  }
  stream << endl;
}

void Table::add (const TableRow& tr) {
  _rows.push_back (tr);

  // update column widths
  TableRow::container::const_iterator
    i = tr._columns.begin (),
    e = tr._columns.end ();
  for (unsigned c = 0; i != e; ++i, ++c) {
    // ensure that _max_width[c] exists
    if (_max_col < c)
      _max_col = c;
    _max_width.resize (_max_col + 1);

    unsigned &max = _max_width[c];
    // FIXME: i18n: screen columns
    unsigned cur = i->length();
    if (max < cur)
      max = cur;
  }
}

// 1st implementation: no width calculation, just tabs
void Table::dumpTo (ostream &stream) const {
  container::const_iterator
    b = _rows.begin (),
    e = _rows.end (),
    i;
  for (i = b; i != e; ++i) {
    i->dumpTo (stream, _max_width);
  }
}
