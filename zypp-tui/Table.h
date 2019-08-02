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

#include <iostream>
#include <sstream>
#include <string>
#include <iosfwd>
#include <set>
#include <list>
#include <vector>

#include <boost/any.hpp>

#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/sat/Solvable.h>
#include <zypp/ui/Selectable.h>

#include "main.h"
#include "utils/ansi.h"
#include "utils/colors.h"

/** Custom sort index type for table rows representing solvables (like detailed search results). */
typedef std::pair<zypp::sat::Solvable, ui::Selectable::picklist_size_type> SolvableCSI;

///////////////////////////////////////////////////////////////////
// Custom sort index helpers
namespace csidetail
{
  /** Default comparator for custom sort indices (std::compare semantic). */
  template <typename T>
  inline int simpleAnyTypeComp ( const boost::any &l_r, const boost::any &r_r )
  {
    T l = boost::any_cast<T>(l_r);
    T r = boost::any_cast<T>(r_r);
    return ( l < r ? -1 : l > r ?  1 : 0 );
  }

  template <>
  inline int simpleAnyTypeComp<SolvableCSI> ( const boost::any &l_r, const boost::any &r_r )
  {
    SolvableCSI l = boost::any_cast<SolvableCSI>(l_r);
    SolvableCSI r = boost::any_cast<SolvableCSI>(r_r);

    if ( l.first == r.first )
      return 0;	// quick check Solvable Id

    int cmp = l.first.name().compare( r.first.name() );
    if ( cmp )
      return cmp;

    cmp = l.first.kind().compare( r.first.kind() );
    if ( cmp )
      return cmp;

    if ( l.second == r.second )
      return 0;
    return ( l.second < r.second ? -1 : 1 );	// `>`! best version up
  }
} // namespace csidetail
///////////////////////////////////////////////////////////////////

//! table drawing style
enum TableLineStyle {
  Ascii		= 0,	///< | - +
  Light,
  Heavy,
  Double,
  Light3,
  Heavy3,
  LightHeavy,
  LightDouble,
  HeavyLight,
  DoubleLight,
  Colon,
  none,
  TLS_End,		///< sentinel
};

class Table;

class TableRow
{
private:
  std::ostream & dumpDetails( std::ostream & stream, const Table & parent ) const;

public:
  TableRow()
  : _ctxt( ColorContext::DEFAULT )
  {}

  explicit TableRow( unsigned c )
  : _ctxt( ColorContext::DEFAULT )
  { _columns.reserve(c); }

  explicit TableRow( ColorContext ctxt_r )
  : _ctxt( ctxt_r )
  {}

  TableRow( unsigned c, ColorContext ctxt_r  )
  : _ctxt( ctxt_r )
  { _columns.reserve(c); }

  TableRow & add( std::string s );

  template<class Tp_>
  TableRow & add( const Tp_ & val_r )
  { return add( asString( val_r ) ); }


  TableRow & addDetail( std::string s );

  template<class Tp_>
  TableRow & addDetail( const Tp_ & val_r )
  { return addDetail( asString( val_r ) ); }


  bool empty() const
  { return _columns.empty(); }

  // return number of columns
  unsigned size() const
  { return _columns.size(); }

  unsigned cols() const
  { return size(); }

  //! tab separated output
  std::ostream & dumbDumpTo( std::ostream & stream ) const;
  //! output with \a parent table attributes
  std::ostream & dumpTo( std::ostream & stream, const Table & parent ) const;

  typedef std::vector<std::string> container;

  const boost::any &userData() const
  { return _userData; }

  void userData( const boost::any &n_r )
  { _userData = n_r; }

  // BinaryPredicate
  struct Less
  {
    std::list<unsigned> _by_columns;
    Less( std::list<unsigned> by_columns_r ) : _by_columns( std::move(by_columns_r) ) {
    }

    bool operator()( const TableRow & a_r, const TableRow & b_r ) const
    {
      for ( unsigned curr_column : _by_columns ) {
        int c = compCol( curr_column, a_r, b_r );
        if ( c > 0 )
          return false;
        else if ( c < 0 )
          return true;
      }
      return false;
    }

    int compCol ( const unsigned curr_column_r, const TableRow & a_r, const TableRow & b_r ) const
    {
      bool noL = curr_column_r >= a_r._columns.size();
      bool noR = curr_column_r >= b_r._columns.size();

      if ( noL || noR ) {
        if ( noL && noR ) {
	  using csidetail::simpleAnyTypeComp;

          const boost::any &lUserData = a_r.userData();
          const boost::any &rUserData = b_r.userData();

          if ( lUserData.empty() && !rUserData.empty() )
            return -1;

          else if ( !lUserData.empty() && rUserData.empty() )
            return 1;

          else if ( lUserData.empty() && rUserData.empty() )
            return 0;

          else if ( lUserData.type() != rUserData.type() ) {
            ZYPP_THROW( zypp::Exception( str::form("Incompatible user types") ) );

          } else if ( lUserData.type() == typeid(SolvableCSI) ) {
            return simpleAnyTypeComp<SolvableCSI> ( lUserData, rUserData );

          } else if ( lUserData.type() == typeid(std::string) ) {
            return simpleAnyTypeComp<std::string>( lUserData, rUserData );

          } else if ( lUserData.type() == typeid(unsigned) ) {
            return simpleAnyTypeComp<unsigned>( lUserData, rUserData );

          } else if ( lUserData.type() == typeid(int) ) {
            return simpleAnyTypeComp<int>( lUserData, rUserData );

          }
          ZYPP_THROW( zypp::Exception( str::form("Unsupported user types") ) );
        } else
          return ( noL && ! noR ? -1 : ! noL && noR ?  1 : 0);
      }
      return ( a_r._columns[curr_column_r] < b_r._columns[curr_column_r] ? -1 : a_r._columns[curr_column_r] > b_r._columns[curr_column_r] ?  1 : 0 );
    }
  };

  const container & columns() const
  { return _columns; }

  container & columns()
  { return _columns; }

private:
  container _columns;
  container _details;
  ColorContext _ctxt;
  boost::any _userData;	///< user defined sort index, e.g. if string values don't work due to coloring
  friend class Table;
};

/** \relates TableRow Add colummn. */
template<class Tp_>
TableRow & operator<<( TableRow & tr, Tp_ && val )
{ return tr.add( asString( std::forward<Tp_>(val) ) ); }
/** \overload preserving TableRow rvalue reference. */
template<class Tp_>
TableRow && operator<<( TableRow && tr, Tp_ && val )
{ return std::move( tr << std::forward<Tp_>(val) ); }


class TableHeader : public TableRow {
public:
  //! Constructor. Reserve place for c columns.
  TableHeader (unsigned c = 0): TableRow (c) {}
};

/** \relates TableHeader  Add column. */
template<class Tp_>
TableHeader & operator<<( TableHeader & th, Tp_ && val )
{ static_cast<TableRow&>(th) << std::forward<Tp_>(val); return th; }
/** \overload preserving TableHeader rvalue reference. */
template<class Tp_>
TableHeader && operator<<( TableHeader && th, Tp_ && val )
{ return std::move( th << std::forward<Tp_>(val) ); }


/** \todo nice idea but poor interface */
class Table
{
public:
  typedef std::list<TableRow> container;

  static TableLineStyle defaultStyle;

  Table & add( TableRow tr );

  Table & setHeader( TableHeader tr );


  std::ostream & dumpTo( std::ostream & stream ) const;
  bool empty() const { return _rows.empty(); }


  /** Unsorted - pseudo sort column indicating not to sort. */
  static constexpr unsigned Unsorted		= unsigned(-1);
  /** UserData - sort column using a custom sort index. */
  static constexpr unsigned UserData		= unsigned(-2);

  /** Get the default sort column or \ref Unsorted (default) */
  unsigned defaultSortColumn() const		{ return _defaultSortColumn; }

  /** Set a \ref defaultSortColumn */
  void defaultSortColumn( unsigned byColumn_r )	{ _defaultSortColumn = byColumn_r; }

  /** Sort by \ref defaultSortColumn */
  void sort()					{ sort( unsigned(_defaultSortColumn ) ); }

  /** Sort by \a byColumn_r */
  void sort( unsigned byColumn_r )		{ if ( byColumn_r != Unsorted ) _rows.sort( TableRow::Less( { byColumn_r } ) ); }
  void sort( const std::list<unsigned> & byColumns_r )	{ if ( byColumns_r.size() ) _rows.sort( TableRow::Less( byColumns_r ) ); }
  void sort( std::list<unsigned> && byColumns_r )	{ if ( byColumns_r.size() ) _rows.sort( TableRow::Less( std::move(byColumns_r) ) ); }

  /** Custom sort */
  template<class TCompare, std::enable_if_t<!std::is_integral<TCompare>::value, int> = 0>
  void sort( TCompare && less_r )		{ _rows.sort( std::forward<TCompare>(less_r) ); }

  void lineStyle( TableLineStyle st );
  void wrap( int force_break_after = -1 );
  void allowAbbrev( unsigned column );
  void margin( unsigned margin );

  const TableHeader & header() const
  { return _header; }
  const container & rows() const
  { return _rows; }
  container & rows()
  { return _rows; }

  Table();

  // poor workaround missing column styles and table entry objects
  void setEditionStyle( unsigned column )
  { _editionStyle.insert( column ); }

private:
  void dumpRule( std::ostream & stream ) const;
  void updateColWidths( const TableRow & tr ) const;

  bool _has_header;
  TableHeader _header;
  container _rows;

  //! maximum column index seen in this table
  mutable unsigned _max_col;
  //! maximum width of respective columns
  mutable std::vector<unsigned> _max_width;
  //! table width (columns)
  mutable int _width;
  //! table line drawing style
  TableLineStyle _style;
  //! amount of space we have to print this table
  int _screen_width;
  //! whether to abbreviate the respective column if needed
  std::vector<bool> _abbrev_col;
  //! left/right margin in number of spaces
  unsigned _margin;
  //! if _do_wrap is set, first break the table at this column;
  //! If negative, wrap as needed.
  int _force_break_after;
  //! Whether to wrap the table if it exceeds _screen_width
  bool _do_wrap;

  DefaultIntegral<unsigned,Unsorted> _defaultSortColumn;

  mutable bool _inHeader;
  std::set<unsigned> _editionStyle;
  bool editionStyle( unsigned column ) const
  { return _editionStyle.find( column ) != _editionStyle.end(); }

  friend class TableRow;
};

namespace table
{
  /** TableHeader manipulator */
  struct EditionStyleSetter
  {
    EditionStyleSetter( Table & table_r, std::string header_r )
    : _table( table_r )
    , _header( std::move(header_r) )
    {}
    Table & _table;
    std::string _header;
  };

  // NOTE: Overloading TableHeader operator<< which uses a universal-reference
  // requires exact matches for the EditionStyleSetter arg! Missing overloads
  // result in compiler complaining about missing EditionStyleSetter::asString.

  /** \relates table::EditionStyleSetter */
  inline TableHeader & operator<<( TableHeader & th, EditionStyleSetter && obj )
  {
    obj._table.setEditionStyle( th.cols() );
    th.add( std::move(obj._header) );
    return th;
  }
  /** \overload preserving TableHeader rvalue reference. */
  inline TableHeader && operator<<( TableHeader && th, EditionStyleSetter && obj )
  { return std::move( th << std::move(obj) ); }
}


inline Table & operator<<( Table & table, TableRow tr )
{ return table.add( std::move(tr) ); }

inline Table & operator<<( Table & table, TableHeader tr )
{ return table.setHeader( std::move(tr) ); }


inline std::ostream & operator<<( std::ostream & stream, const Table & table )
{ return table.dumpTo( stream ); }


///////////////////////////////////////////////////////////////////
/// \class PropertyTable
/// \brief Aligned key/value with multiline support
/// Key       : value 1
/// LongKey   : value 2
/// Multiline :
///     line 1
///     line 2
/// Next Key  : value 3
///
///////////////////////////////////////////////////////////////////
class PropertyTable
{
public:
  PropertyTable()
  { _table.lineStyle( ::Colon ); }

  static const char * emptyListTag() { return "---"; }

public:
  ///////////////////////////////////////////////////////////////////
  // Key / Value
  template <class KeyType>
  PropertyTable & add( const KeyType & key_r )
  { _table << ( TableRow() << key_r << "" ); return *this; }

  template <class KeyType, class ValueType>
  PropertyTable & add( const KeyType & key_r, const ValueType & val_r )
  { _table << ( TableRow() << key_r << val_r ); return *this; }

  template <class KeyType>
  PropertyTable & add( const KeyType & key_r, bool val_r )
  { _table << ( TableRow() << key_r << asYesNo( val_r ) ); return *this; }

  ///////////////////////////////////////////////////////////////////
  // Key / Value in details (e.g. Description:)
  template <class ValueType>
  PropertyTable & addDetail( const ValueType & val_r )
  { last().addDetail( val_r ); return *this; }

  template <class KeyType, class ValueType>
  PropertyTable & addDetail( const KeyType & key_r, const ValueType & val_r )
  { _table << ( TableRow() << key_r << "" ).addDetail( val_r ); return *this; }

  ///////////////////////////////////////////////////////////////////
  // Key / Container<Value>
  template <class KeyType, class Iterator_ >
  PropertyTable & add( const KeyType & key_r, Iterator_ begin_r, Iterator_ end_r, bool forceDetails_r = false  )
  {
    TableRow r;
    r << key_r;
    if ( begin_r != end_r )
    {
      unsigned cnt = 1;
      Iterator_ first = begin_r++;
      if ( begin_r == end_r && ! forceDetails_r )
	r << *first;				// only one value
      else
      {
	r.addDetail( *first );			// list all in details
	while ( begin_r != end_r )
	{
	  ++cnt;
	  r.addDetail( *(begin_r++) );
	}
	r << "["+str::numstring(cnt)+"]";	// size as value
      }
   }
    else
    { r << emptyListTag(); }				// dummy to get the ":" if empty
    _table << r;
    return *this;
  }

  template <class KeyType, class ContainerType>
  PropertyTable & lst( const KeyType & key_r, const ContainerType & lst_r, bool forceDetails_r = false )
  { return add( key_r, lst_r.begin(), lst_r.end(), forceDetails_r ); }

  template <class KeyType, class ValueType>
  PropertyTable & add( const KeyType & key_r, const std::set<ValueType> & lst_r, bool forceDetails_r = false  )
  { return lst( key_r, lst_r, forceDetails_r );  }
  template <class KeyType, class ValueType>
  PropertyTable & add( const KeyType & key_r, const std::list<ValueType> & lst_r, bool forceDetails_r = false  )
  { return lst( key_r, lst_r, forceDetails_r );  }
  template <class KeyType, class ValueType>
  PropertyTable & add( const KeyType & key_r, const std::vector<ValueType> & lst_r, bool forceDetails_r = false  )
  { return lst( key_r, lst_r, forceDetails_r ); }

  ///////////////////////////////////////////////////////////////////
  // misc
  PropertyTable & paint( ansi::Color color_r, bool cond_r = true )
  {
    if ( cond_r )
    {
      // FIXME re-coloring like this works only once
      std::string & lastval( _table.rows().back().columns().back() );
      lastval = ColorString( lastval, color_r ).str();
    }
    return *this;
  }

  TableRow & last()
  { return _table.rows().back(); }

  std::string & lastKey()
  { return last().columns()[0]; }

  std::string & lastValue()
  { return last().columns()[1]; }

public:
  friend std::ostream & operator << ( std::ostream & str, const PropertyTable & obj )
  { return str << obj._table; }

private:
  Table _table;
};



// Local Variables:
// c-basic-offset: 2
// End:
#endif
