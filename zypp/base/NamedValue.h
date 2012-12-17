/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/NamedValue.h
 *
*/
#ifndef ZYPP_BASE_NAMEDVALUE_H
#define ZYPP_BASE_NAMEDVALUE_H

#include <stdexcept>
#include <string>
#include <map>

///////////////////////////////////////////////////////////////////
namespace zypp
{

  ///////////////////////////////////////////////////////////////////
  /// \class NamedValue<_Tp>
  /// \brief Simple value<>name mapping supporting aliases.
  /// \code
  ///   enum Commands {
  ///     CMD_1,
  ///     CMD_2
  ///   };
  ///   NamedValue<Commands> clist;
  ///   //     Value   | Name   | Alias...
  ///   clist( CMD_1 ) | "cmd1";
  ///   clist( CMD_2 ) | "cmd2" | "second";
  ///
  ///   std::string name( clist.getName( CMD_1 ) );
  ///   Commands    cmd( clist.getValue( "second" ) );
  /// \endcode
  ///////////////////////////////////////////////////////////////////
  template< class _Tp, const bool _WithAlias = true >
  class NamedValue
  {
    typedef std::map< std::string, _Tp > NameMap;
    typedef std::map< _Tp, std::string > ValueMap;

  public:
    /** Wheter not initialized (no (name,value) pair remembered) */
    bool empty() const
    { return( _nameMap.empty() && _valueMap.empty() ); }

  public:
    /** \name Get value for name or alias. */
    //@{
      /** Whether there is a \c value mapped for \a name_r.
       */
      bool haveValue( const std::string & name_r ) const
      {
	typename NameMap::const_iterator it( _nameMap.find( name_r ) );
	return( it != _nameMap.end() );
      }

      /** Get value mapped for name or alias.
       * \return \c true if name or alias was found.
       */
      bool getValue( const std::string & name_r, _Tp & value_r ) const
      {
	typename NameMap::const_iterator it( _nameMap.find( name_r ) );
	if ( it == _nameMap.end() )
	  return false;
	value_r = it->second;
	return true;
      }
      /** \overload \throws std::out_of_range exception if \a name_r was not found. */
      const _Tp & getValue( const std::string & name_r ) const
      { return _nameMap.at( name_r ); }
    //@}


    /** \name Get name for value. */
    //@{
      /** Whether there is a \c name mapped for \a value_r.
       */
      bool haveName( const std::string & value_r ) const
      {
	typename ValueMap::const_iterator it( _valueMap.find( value_r ) );
	return( it != _valueMap.end() );
      }

      /** Get name of value.
       * \return \c true if name or alias was found.
       */
      bool getName( const _Tp & value_r, std::string & name_r ) const
      {
	typename ValueMap::const_iterator it( _valueMap.find( value_r ) );
	if ( it == _valueMap.end() )
	  return false;
	value_r = it->second;
	return true;
      }
      /** \overload \throws std::out_of_range exception if \a value_r was not found. */
      const std::string & getName( const _Tp & value_r ) const
      { return _valueMap.at( value_r ); }
    //@}

  public:
    /** \name Inserter
     */
    //@{
      class _Inserter
      {
      public:
	_Inserter( NamedValue & parent_r, const _Tp & value_r )
	: _parent( &parent_r )
	, _value( value_r )
	{}
	_Inserter & operator|( const std::string & name_r )
	{ _parent->insert( _value, name_r ); return *this; }
      private:
	NamedValue * _parent;
	_Tp _value;
      };

      _Inserter operator()( const _Tp & value_r )
      { return _Inserter( *this, value_r ); }
    //@}

    /** Remember name (1st call) or alias (subsequent calls).
     * \return \C true if this is the 1st call for \a value_r.
     * \throws std::logic_error if \a name_r is already used as name or alias.
     * \throws std::logic_error if \c _WithAlias is \c false and a name for \a value_r is already defined.
     */
    bool insert( const _Tp & value_r, const std::string & name_r )
    {
      typename NameMap::const_iterator nit( _nameMap.find( name_r ) );
      if ( nit != _nameMap.end() )	// duplicate name
	throw std::logic_error( "NamedValue::insert name" );

      typename ValueMap::const_iterator tit( _valueMap.find( value_r ) );
      if ( tit != _valueMap.end() )	// duplicate value, i.e. an alias
      {
	if ( !_WithAlias )
	  throw std::logic_error( "NamedValue::insert alias" );

	_nameMap[name_r] = value_r;
	return false;
      }
      // here: 1st entry for value_r
      _nameMap[name_r] = value_r;
      _valueMap[value_r] = name_r;
      return true;
    }

  private:
    NameMap _nameMap;
    ValueMap _valueMap;
  };
  ///////////////////////////////////////////////////////////////////

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_NAMEDVALUE_H
