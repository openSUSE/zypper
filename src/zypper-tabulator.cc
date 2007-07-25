#include <iostream>
#include "zypper-tabulator.h"
using namespace std;

TableStyle Table::defaultStyle = Ascii;

static
const char * lines[][3] = {
  { "|", "-", "+"},		///< Ascii
  // utf 8. TODO wchars?
  { "\xE2\x94\x82", "\xE2\x94\x80", "\xE2\x94\xBC"}, ///< light
  { "\xE2\x94\x83", "\xE2\x94\x81", "\xE2\x95\x8B"}, ///< heavy
  { "\xE2\x95\x91", "\xE2\x95\x90", "\xE2\x95\xAC"}, ///< double
  { "\xE2\x94\x86", "\xE2\x94\x84", "\xE2\x94\xBC"}, ///< light 3
  { "\xE2\x94\x87", "\xE2\x94\x85", "\xE2\x94\x8B"}, ///< heavy 3
  { "\xE2\x94\x82", "\xE2\x94\x81", "\xE2\x94\xBF"}, ///< v light, h heavy
  { "\xE2\x94\x82", "\xE2\x95\x90", "\xE2\x95\xAA"}, ///< v light, h double
  { "\xE2\x94\x83", "\xE2\x94\x80", "\xE2\x95\x82"}, ///< v heavy, h light
  { "\xE2\x95\x91", "\xE2\x94\x80", "\xE2\x95\xAB"}, ///< v double, h light
};

void TableRow::add (const string& s) {
  _columns.push_back (s);
}

unsigned int TableRow::cols( void ) const {
  return _columns.size();
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

void TableRow::dumpTo (ostream &stream, const vector<unsigned>& widths,
		       TableStyle st) const {
  const char * vline = lines[st][0];

  bool seen_first = false;
  container::const_iterator
    i = _columns.begin (),
    e = _columns.end ();
  
  stream.setf (ios::left, ios::adjustfield);
  for (unsigned c = 0; i != e ; ++i, ++c) {
    if (seen_first) {
      stream.width (0);
      // pad vertical line with spaces
      stream << ' ' << vline << ' ';
    }
    seen_first = true;

    stream.width (widths[c]);
    stream << *i;
  }
  stream << endl;
}

void Table::add (const TableRow& tr) {
  _rows.push_back (tr);
  updateColWidths (tr);
}

void Table::setHeader (const TableHeader& tr) {
  _has_header = true;
  _header = tr;
  updateColWidths (tr);
}

void Table::updateColWidths (const TableRow& tr) {
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

void Table::dumpRule (ostream &stream) const {
  const char * hline = lines[_style][1];
  const char * cross = lines[_style][2];

  bool seen_first = false;
  
  stream.width (0);
  for (unsigned c = 0; c <= _max_col; ++c) {
    if (seen_first) {
      stream << hline << cross << hline;
    }
    seen_first = true;
    // FIXME: could use fill character if hline were a (wide) character
    for (unsigned i = 0; i < _max_width[c]; ++i) {
      stream << hline;
    }
  }
  stream << endl;
}

void Table::dumpTo (ostream &stream) const {
  if (_has_header) {
    _header.dumpTo (stream, _max_width, _style);
    dumpRule (stream);
  }

  container::const_iterator
    b = _rows.begin (),
    e = _rows.end (),
    i;
  for (i = b; i != e; ++i) {
    i->dumpTo (stream, _max_width, _style);
  }
}

void Table::style (TableStyle st) {
  if (st < _End)
    _style = st;
}

void Table::sort (unsigned by_column) {
  if (by_column >= _max_col) {
#warning Provide proper error handling here
    return;
  }

  TableRow::Less comp (by_column);
  _rows.sort (comp);
}

// Local Variables:
// c-basic-offset: 2
// End:
