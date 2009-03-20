#include <iostream>
#include <cstring>
#include <cstdlib>
#include <readline/readline.h>
#include <wchar.h>

#include "zypp/base/Logger.h"

#include "Table.h"

// libzypp logger settings
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper"

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

// A non-ASCII string has 3 different lengths:
// - bytes
// - characters (non-ASCII ones have multiple bytes in UTF-8)
// - columns (Chinese characters are 2 columns wide)
// In #328918 see how confusing these leads to misalignment.

// return the number of columns in str, or -1 if there's an error
static
int string_to_columns_e (const string& str) {
  // from smpppd.src.rpm/format.cc, thanks arvin

  const char* ptr = str.c_str ();
  size_t s_bytes = str.length ();
  int s_cols = 0;

  mbstate_t shift_state;
  memset (&shift_state, 0, sizeof (shift_state));

  wchar_t wc;
  size_t c_bytes;

  // mbrtowc produces one wide character from a multibyte string
  while ((c_bytes = mbrtowc (&wc, ptr, s_bytes, &shift_state)) > 0) {
    if (c_bytes >= (size_t) -2) // incomplete (-2) or invalid (-1) sequence
      return -1;

    s_cols += wcwidth (wc);

    s_bytes -= c_bytes;
    ptr += c_bytes;
  }

  return s_cols;
}

static
unsigned string_to_columns (const string& str) {
  int c = string_to_columns_e (str);
  if (c < 0)
    return str.length();	// fallback if there was an error
  else
    return (unsigned) c;
}

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

  unsigned int ssize = 0; // string size in columns
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

    // stream.width (widths[c]); // that does not work with multibyte chars
    const string & s = *i;
    ssize = string_to_columns (s);
    if (ssize > widths[c])
      stream << (s.substr(0, widths[c] - 2) + "->"); //! \todo FIXME cut at the correct place
    else
    {
      stream << s;
      stream.width (widths[c] - ssize);
    }
    stream << "";
  }
  stream << endl;
}

// ----------------------( Table )---------------------------------------------

Table::Table() :
  _has_header (false),
  _max_col (0),
  _width(0),
  _style (defaultStyle)
{
  //! \todo move this to utils

  const char *cols_env = getenv("COLUMNS");
  if (cols_env)
    _screen_width  = ::atoi (cols_env);
  else
  {
    ::rl_initialize();
    //::rl_reset_screen_size();
    ::rl_get_screen_size (NULL, &_screen_width);
    DBG << "readline says we have " << _screen_width << " char wide console screen" << endl;
  }

  // safe default
  if (!_screen_width)
     _screen_width = 80;

  DBG << "got screen width of " << _screen_width << endl;
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

void Table::allowAbbrev(unsigned column) {
  if (column >= _abbrev_col.size()) {
    _abbrev_col.reserve(column + 1);
    _abbrev_col.insert(_abbrev_col.end(), column - _abbrev_col.size() + 1, false);
  }
  _abbrev_col[column] = true;
}

void Table::updateColWidths (const TableRow& tr) {
  _width = -3;

  TableRow::container::const_iterator
    i = tr._columns.begin (),
    e = tr._columns.end ();
  for (unsigned c = 0; i != e; ++i, ++c) {
    // ensure that _max_width[c] exists
    if (_max_col < c)
      _max_col = c;
    _max_width.resize (_max_col + 1);

    unsigned &max = _max_width[c];
    unsigned cur = string_to_columns (*i);

    if (max < cur)
      max = cur;

    _width += max + 3;
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

  // reset column widths for columns that can be abbreviated
  //! \todo allow abbrev of multiple columns?
  unsigned c = 0;
  for (vector<bool>::const_iterator it = _abbrev_col.begin();
      it != _abbrev_col.end() && c <= _max_col; ++it, ++c) {
    if (*it && _width > _screen_width) {
      _max_width[c] -= _width - _screen_width;
      break;
    }
  }

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
  if (by_column > _max_col) {
    ERR << "by_column >= _max_col (" << by_column << ">=" << _max_col << ")" << endl;
    return;
  }

  TableRow::Less comp (by_column);
  _rows.sort (comp);
}

// Local Variables:
// c-basic-offset: 2
// End:
