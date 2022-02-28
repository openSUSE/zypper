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

///////////////////////////////////////////////////////////////////
// Conditional Table column helpers.
namespace ctcdetail
{
  /// Remember either \a _if or \a _else function.
  template<class Tif_,class Telse_>
  struct ColumnIf
  {
    ColumnIf( bool condition_r, std::function<Tif_()> if_r,  std::function<Telse_()> else_r )
    { if ( condition_r ) _if = std::move(if_r); else _else = std::move(else_r); }
    std::function<Tif_()> _if;
    std::function<Telse_()> _else;
  };
  /// Specialization both functions return the same type
  template<class Tif_>
  struct ColumnIf<Tif_,Tif_>
  {
    ColumnIf( bool condition_r, std::function<Tif_()> if_r,  std::function<Tif_()> else_r )
    : _ifelse { condition_r ? std::move(if_r) : std::move(else_r) }
    {}
    ColumnIf( bool condition_r, std::function<Tif_()> && if_r )
    { if ( condition_r ) _ifelse = std::move(if_r); }
    std::function<Tif_()> _ifelse;
  };
}
///////////////////////////////////////////////////////////////////
/// Conditional Table column factory.
///
/// Creates an conditional \ref TableRow or \ref TableHeader entry,
/// depending on the value of \a condition_r. The columns content,
/// if needed, is determined calling the function object matching the
/// condition.
///
/// An empty function object will cause the column to be hidden and no
/// content will be retrieved (see overloaded operator<<(TableRow &,..)).
///
/// \code
///   TableRow()
///     << "first"
///     << ColumnIf( condition, [](){ return "second"; }, [](){ return "one but last"; } )
///     << "last";
/// \endcode
template<class Tif_, class Telse_>
auto ColumnIf( bool condition_r, Tif_ && if_r, Telse_ && else_r ) -> ctcdetail::ColumnIf<decltype(if_r()),decltype(else_r())>
{ return { condition_r, std::forward<Tif_>(if_r), std::forward<Telse_>(else_r) }; }
/** \overload no column on 'else' */
template<class Tif_>
auto ColumnIf( bool condition_r, Tif_ && if_r ) -> ctcdetail::ColumnIf<decltype(if_r()),decltype(if_r())>
{ return { condition_r, std::forward<Tif_>(if_r) }; }


namespace table
{
  /// Table column styles
  ///
  /// \see \ref Column style setter below.
  /// \code
  /// th << table::Column( N_("Name"), CStyle::SortCi );
  /// \endcode
  enum class CStyle
  {
    Default = 0,
    Edition,      ///< Editions with v-r setparator highlighted
    SortCi,       ///< String values to be sorted case insensitive
  };
}


class TableRow
{
private:
  std::ostream & dumpDetails( std::ostream & stream, const Table & parent ) const;

public:
  struct Less; ///< Binary predicate for sorting.

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

  const container & columns() const
  { return _translateColumns ? _translatedColumns : _columns; }

  container & columns()
  { return _translateColumns ? _translatedColumns : _columns; }

  const container & columnsNoTr() const
  { return _columns; }

  container & columnsNoTr()
  { return _columns; }

protected:
  bool      _translateColumns = false;
private:
  container _columns;
  container _translatedColumns;
  container _details;
  ColorContext _ctxt;
  boost::any _userData;	///< user defined sort index, e.g. if string values don't work due to coloring
};

/** \relates TableRow Add colummn. */
template<class Tp_>
TableRow & operator<<( TableRow & tr, Tp_ && val )
{ return tr.add( asString( std::forward<Tp_>(val) ) ); }
/** \overload preserving TableRow rvalue reference. */
template<class Tp_>
TableRow && operator<<( TableRow && tr, Tp_ && val )
{ return std::move( tr << std::forward<Tp_>(val) ); }

/** \overload universal refernce for conditional Table columns */
template<class Tif_, class Telse_> TableRow & operator<<( TableRow & tr, const ctcdetail::ColumnIf<Tif_,Telse_> & val )
{ if ( val._if ) tr.add( val._if() ); else if ( val._else ) tr.add( val._else() ); return tr; }
/** \overload universal refernce for conditional Table columns */
template<class Tif_, class Telse_> TableRow & operator<<( TableRow & tr, ctcdetail::ColumnIf<Tif_,Telse_> & val )
{ if ( val._if ) tr.add( val._if() ); else if ( val._else ) tr.add( val._else() ); return tr; }
/** \overload universal refernce for conditional Table columns */
template<class Tif_, class Telse_> TableRow & operator<<( TableRow & tr, ctcdetail::ColumnIf<Tif_,Telse_> && val )
{ if ( val._if ) tr.add( val._if() ); else if ( val._else ) tr.add( val._else() ); return tr; }

/** \overload universal refernce for conditional Table columns (specialized) */
template<class Tif_> TableRow & operator<<( TableRow & tr, const ctcdetail::ColumnIf<Tif_,Tif_> & val )
{ if ( val._ifelse ) tr.add( val._ifelse() ); return tr; }
/** \overload universal refernce for conditional Table columns (specialized) */
template<class Tif_> TableRow & operator<<( TableRow & tr, ctcdetail::ColumnIf<Tif_,Tif_> & val )
{ if ( val._ifelse ) tr.add( val._ifelse() ); return tr; }
/** \overload universal refernce for conditional Table columns (specialized) */
template<class Tif_> TableRow & operator<<( TableRow & tr, ctcdetail::ColumnIf<Tif_,Tif_> && val )
{ if ( val._ifelse ) tr.add( val._ifelse() ); return tr; }


class TableHeader : public TableRow
{
public:
  using CStyle = table::CStyle;
  //! Constructor. Reserve place for c columns.
  TableHeader (unsigned c = 0): TableRow (c) { _translateColumns = true; }

  bool hasStyle( unsigned c, CStyle s ) const
  { return _cstyle.count(c) && _cstyle.at(c) == s; }

  CStyle style( unsigned c ) const
  { return _cstyle.count(c) ? _cstyle.at(c) : CStyle::Default; }

  void style( unsigned c, CStyle s )
  { _cstyle[c] = s; }


  std::set<unsigned> editionColumns() const
  {
    std::set<unsigned> ret;
    for ( const auto & [c,s] : _cstyle ) {
      if ( s == CStyle::Edition )
        ret.insert( c );
    }
    return ret;
  }

private:
  std::map<unsigned,CStyle> _cstyle;  ///< Column style and sort hints are remembered here
};

/** \relates TableHeader  Add column. */
template<class Tp_>
TableHeader & operator<<( TableHeader & th, Tp_ && val )
{ static_cast<TableRow&>(th) << std::forward<Tp_>(val); return th; }
/** \overload preserving TableHeader rvalue reference. */
template<class Tp_>
TableHeader && operator<<( TableHeader && th, Tp_ && val )
{ return std::move( th << std::forward<Tp_>(val) ); }


struct TableRow::Less
{
  using SortParam = std::tuple<unsigned,bool>;  ///< column and sortCI

  Less( const TableHeader & header_r, std::list<unsigned> by_columns_r )
  {
    for ( unsigned curr_column : by_columns_r ) {
      _by_columns.push_back( SortParam( curr_column, header_r.hasStyle( curr_column, table::CStyle::SortCi ) ) );
    }
  }

  bool operator()( const TableRow & a_r, const TableRow & b_r ) const
  {
    int c = 0;
    for ( const SortParam sortParam : _by_columns ) {
      if ( (c = compCol( sortParam, a_r, b_r )) )
        return c < 0;
    }
    return false;
  }

private:
  int compCol( const SortParam & sortParam_r, const TableRow & a_r, const TableRow & b_r ) const
  {
    const auto & [ byColumn, sortCI ] { sortParam_r };
    bool noL = byColumn >= a_r._columns.size();
    bool noR = byColumn >= b_r._columns.size();

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
    return defaultStrComp( sortCI, a_r._columns[byColumn], b_r._columns[byColumn] );
  }

  /** Natural('sort -V' like) [case insensitive] compare ignoring ANSI SGR chars. */
  static int defaultStrComp( bool ci_r, const std::string & lhs, const std::string & rhs );

private:
  std::list<SortParam> _by_columns;
};

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
  void sort( unsigned byColumn_r )		        { if ( byColumn_r != Unsorted ) _rows.sort( TableRow::Less( header(), { byColumn_r } ) ); }
  void sort( const std::list<unsigned> & byColumns_r )	{ if ( byColumns_r.size() ) _rows.sort( TableRow::Less( header(), byColumns_r ) ); }
  void sort( std::list<unsigned> && byColumns_r )	{ if ( byColumns_r.size() ) _rows.sort( TableRow::Less( header(), std::move(byColumns_r) ) ); }

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

  friend class TableRow;
};

namespace table
{
  /// Table column style setter.
  ///
  /// \see \ref CStyle colum styles.
  /// \code
  /// th << table::Column( N_("Name"), CStyle::SortCi );
  /// \endcode
  struct Column
  {
    Column( std::string header_r, CStyle style_r = CStyle::Default )
    : _header( std::move(header_r) )
    , _style( style_r )
    {}
    std::string _header;
    CStyle      _style;
  };
  /** \relates table::Column set \ref TableHeader style. */
  inline TableHeader & operator<<( TableHeader & th, Column && obj )
  { th.style( th.cols(), obj._style ); return th << std::move(obj._header); }
  /** \relates table::Column set \ref TableHeader style.*/
  inline TableHeader && operator<<( TableHeader && th, Column && obj )
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
