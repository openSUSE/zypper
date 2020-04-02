#include <iostream>
#include <cstring>
#include <cstdlib>

#include <zypp/base/LogTools.h>
#include <zypp/base/String.h>
#include <zypp/base/DtorReset.h>

#include "utils/colors.h"
#include "utils/console.h"
#include "utils/text.h"

#include "Zypper.h"
#include "Table.h"

// libzypp logger settings
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypper"


TableLineStyle Table::defaultStyle = Ascii;

static const char * lines[][3] = {
  { "|", "-", "+"},		///< Ascii
  // utf 8
  { "\xE2\x94\x82", "\xE2\x94\x80", "\xE2\x94\xBC" },	///< light
  { "\xE2\x94\x83", "\xE2\x94\x81", "\xE2\x95\x8B" },	///< heavy
  { "\xE2\x95\x91", "\xE2\x95\x90", "\xE2\x95\xAC" },	///< double
  { "\xE2\x94\x86", "\xE2\x94\x84", "\xE2\x94\xBC" },	///< light 3
  { "\xE2\x94\x87", "\xE2\x94\x85", "\xE2\x94\x8B" },	///< heavy 3
  { "\xE2\x94\x82", "\xE2\x94\x81", "\xE2\x94\xBF" },	///< v light, h heavy
  { "\xE2\x94\x82", "\xE2\x95\x90", "\xE2\x95\xAA" },	///< v light, h double
  { "\xE2\x94\x83", "\xE2\x94\x80", "\xE2\x95\x82" },	///< v heavy, h light
  { "\xE2\x95\x91", "\xE2\x94\x80", "\xE2\x95\xAB" },	///< v double, h light
  { ":", "-", "+" },					///< colon separated values
};

TableRow & TableRow::add( std::string s )
{
  _columns.push_back( std::move(s) );
  return *this;
}

TableRow & TableRow::addDetail( std::string s )
{
  _details.push_back( std::move(s) );
  return *this;
}

// 1st implementation: no width calculation, just tabs
std::ostream & TableRow::dumbDumpTo( std::ostream & stream ) const
{
  bool seen_first = false;
  for ( container::const_iterator i = _columns.begin(); i != _columns.end(); ++i )
  {
    if ( seen_first )
      stream << '\t';
    seen_first = true;

    stream << *i;
  }
  return stream << endl;
}

std::ostream & TableRow::dumpDetails( std::ostream & stream, const Table & parent ) const
{
  mbs::MbsWriteWrapped mww( stream, 4, parent._screen_width );
  for ( const std::string & text : _details )
  {
    mww.writePar( text );
  }
  mww.gotoParBegin();
  return stream;
}

std::ostream & TableRow::dumpTo( std::ostream & stream, const Table & parent ) const
{
  const char * vline = parent._style == none ? "" : lines[parent._style][0];

  unsigned ssize = 0; // string size in columns
  bool seen_first = false;

  stream.setf( std::ios::left, std::ios::adjustfield );
  stream << std::string( parent._margin, ' ' );
  // current position at currently printed line
  int curpos = parent._margin;
  // On a table with 2 edition columns highlight the editions
  // except for the common prefix.
  std::string::size_type editionSep( std::string::npos );

  container::const_iterator i = _columns.begin (), e = _columns.end ();
  const unsigned lastCol = _columns.size() - 1;
  for ( unsigned c = 0; i != e ; ++i, ++c )
  {
    const std::string & s( *i );

    if ( seen_first )
    {
      bool do_wrap = parent._do_wrap				// user requested wrapping
		  && parent._width > parent._screen_width	// table is wider than screen
		  && ( curpos + (int)parent._max_width[c] + (parent._style == none ? 2 : 3) > parent._screen_width	// the next table column would exceed the screen size
		    || parent._force_break_after == (int)(c - 1) );	// or the user wishes to first break after the previous column

      if ( do_wrap )
      {
        // start printing the next table columns to new line,
        // indent by 2 console columns
        stream << endl << std::string( parent._margin + 2, ' ' );
        curpos = parent._margin + 2; // indent == 2
      }
      else
        // vertical line, padded with spaces
        stream << ' ' << vline << ' ';
      stream.width( 0 );
    }
    else
      seen_first = true;

    // stream.width (widths[c]); // that does not work with multibyte chars
    ssize = mbs_width( s );
    if ( ssize > parent._max_width[c] )
    {
      unsigned cutby = parent._max_width[c] - 2;
      std::string cutstr = mbs_substr_by_width( s, 0, cutby );
      stream << ( _ctxt << cutstr ) << std::string(cutby - mbs_width( cutstr ), ' ') << "->";
    }
    else
    {
      if ( !parent._inHeader && parent.editionStyle( c ) && Zypper::instance().config().do_colors )
      {
	// Edition column
	if ( parent._editionStyle.size() == 2 )
	{
	  // 2 Edition columns - highlight difference
	  if ( editionSep == std::string::npos )
	  {
	    editionSep = str::commonPrefix( _columns[*parent._editionStyle.begin()],
						  _columns[*(++parent._editionStyle.begin())] );
	  }

	  if ( editionSep == 0 )
	  {
	    stream << ( ColorContext::CHANGE << s );
	  }
	  else if ( editionSep == s.size() )
	  {
	    stream << ( _ctxt << s );
	  }
	  else
	  {
	    stream << ( _ctxt << s.substr( 0, editionSep ) ) << ( ColorContext::CHANGE << s.substr( editionSep ) );
	  }
	}
	else
	{
	  // highlight edition-release separator
	  editionSep = s.find( '-' );
	  if ( editionSep != std::string::npos )
	  {
	    stream << ( _ctxt << s.substr( 0, editionSep ) << ( ColorContext::HIGHLIGHT << "-" ) << s.substr( editionSep+1 ) );
	  }
	  else	// no release part
	  {
	    stream << ( _ctxt << s );
	  }
	}
      }
      else	// no special style
      {
	stream << ( _ctxt << s );
      }
      stream.width( c == lastCol ? 0 : parent._max_width[c] - ssize );
    }
    stream << "";
    curpos += parent._max_width[c] + (parent._style == none ? 2 : 3);
  }
  stream << endl;

  if ( !_details.empty() )
  {
    dumpDetails( stream, parent );
  }
  return stream;
}

// ----------------------( Table )---------------------------------------------

Table::Table()
  : _has_header( false )
  , _max_col( 0 )
  , _max_width( 1, 0 )
  , _width( 0 )
  , _style( defaultStyle )
  , _screen_width( get_screen_width() )
  , _margin( 0 )
  , _force_break_after( -1 )
  , _do_wrap( false )
  , _inHeader( false )
{}

Table & Table::add( TableRow tr )
{
  _rows.push_back( std::move(tr) );
  return *this;
}

Table & Table::setHeader( TableHeader tr )
{
  _header = std::move(tr);
  _has_header = !_header.empty();
  return *this;
}

void Table::allowAbbrev( unsigned column)
{
  if ( column >= _abbrev_col.size() )
  {
    _abbrev_col.reserve( column + 1 );
    _abbrev_col.insert( _abbrev_col.end(), column - _abbrev_col.size() + 1, false );
  }
  _abbrev_col[column] = true;
}

void Table::updateColWidths( const TableRow & tr ) const
{
  // how much columns spearators add to the width of the table
  int sepwidth = _style == none ? 2 : 3;
  // initialize the width to -sepwidth (the first column does not have a line
  // on the left)
  _width = -sepwidth;

  // ensure that _max_width[col] exists
  if ( _max_width.size() < tr._columns.size() )
  {
    _max_width.resize( tr._columns.size(), 0 );
    _max_col = _max_width.size()-1;
  }

  unsigned c = 0;
  for ( const auto & col : tr._columns )
  {
    unsigned &max = _max_width[c++];
    unsigned cur = mbs_width( col );

    if ( max < cur )
      max = cur;

    _width += max + sepwidth;
  }
  _width += _margin * 2;
}

void Table::dumpRule( std::ostream &stream ) const
{
  const char * hline = _style != none ? lines[_style][1] : " ";
  const char * cross = _style != none ? lines[_style][2] : " ";

  bool seen_first = false;

  stream.width( 0 );
  stream << std::string(_margin, ' ' );
  for ( unsigned c = 0; c <= _max_col; ++c )
  {
    if ( seen_first )
      stream << hline << cross << hline;
    seen_first = true;
    // FIXME: could use fill character if hline were a (wide) character
    for ( unsigned i = 0; i < _max_width[c]; ++i )
      stream << hline;
  }
  stream << endl;
}

std::ostream & Table::dumpTo( std::ostream & stream ) const
{
  // compute column sizes
  if ( _has_header )
    updateColWidths( _header );
  for ( const auto & row : _rows )
    updateColWidths( row );

  // reset column widths for columns that can be abbreviated
  //! \todo allow abbrev of multiple columns?
  unsigned c = 0;
  for ( std::vector<bool>::const_iterator it = _abbrev_col.begin(); it != _abbrev_col.end() && c <= _max_col; ++it, ++c )
  {
    if ( *it && _width > _screen_width &&
         // don't resize the column to less than 3, or if the resulting table
         // would still exceed the screen width (bnc #534795)
         _max_width[c] > 3 &&
         _width - _screen_width < ((int) _max_width[c]) - 3 )
    {
      _max_width[c] -= _width - _screen_width;
      break;
    }
  }

  if ( _has_header )
  {
    DtorReset inHeader( _inHeader, false );
    _inHeader = true;
    _header.dumpTo( stream, *this );
    dumpRule (stream);
  }

  for ( const auto & row : _rows )
    row.dumpTo( stream, *this );

  return stream;
}

void Table::wrap( int force_break_after )
{
  if ( force_break_after >= 0 )
    _force_break_after = force_break_after;
  _do_wrap = true;
}

void Table::lineStyle( TableLineStyle st )
{
  if ( st < TLS_End )
    _style = st;
}

void Table::margin( unsigned margin )
{
  if ( margin < (unsigned)(_screen_width/2) )
    _margin = margin;
  else
    ERR << "margin of " << margin << " is greater than half of the screen" << endl;
}

// Local Variables:
// c-basic-offset: 2
// End:
