/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/UserData.h
 */
#ifndef ZYPP_USERDATA_H
#define ZYPP_USERDATA_H

#include <iosfwd>
#include <string>
#include <map>
#include <boost/any.hpp>

#include "zypp/base/PtrTypes.h"
#include "zypp/ContentType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace callback
  {
    ///////////////////////////////////////////////////////////////////
    /// \class UserData
    /// \brief Typesafe passing of user data via callbacks
    ///
    /// Basically a <tt>std::map<std::string,boost::any></tt> plus
    /// associated \ref ContentType.
    ///
    /// Constness protects non-empty values from being modified.
    /// It is possible to overwrite empty values or to add new ones.
    ///////////////////////////////////////////////////////////////////
    class UserData
    {
      typedef std::map<std::string,boost::any> DataType;
      typedef DataType::size_type size_type;
      typedef DataType::key_type key_type;
      typedef DataType::value_type value_type;
      typedef DataType::const_iterator const_iterator;
    public:
      /** Default ctor. */
      UserData()
      {}

      /** Ctor taking ContentType. */
      explicit UserData( ContentType type_r )
      : _type( std::move(type_r) )
      {}
      /** Ctor taking ContentType. */
      explicit UserData( std::string type_r )
      : UserData( ContentType( std::move(type_r) ) )
      {}
      /** Ctor taking ContentType. */
      UserData( std::string type_r, std::string subtype_r )
      : UserData( ContentType( std::move(type_r), std::move(subtype_r) ) )
      {}

    public:
      /** Get type. */
      const ContentType & type() const
      { return _type; }

      /** Set type. */
      void type( ContentType type_r )
      { _type = std::move(type_r); }

    public:
       /**  Validate object in a boolean context: has data */
      explicit operator bool() const
      { return !empty(); }

      /** Whether \ref data is empty. */
      bool empty() const
      { return !_dataP || _dataP->empty(); }

      /** Size of \ref data. */
      size_type size() const
      { return _dataP ? _dataP->size() : 0;  }

      /** The \ref data. */
      const DataType & data() const
      { return dataRef(); }

      /** Whether \a key_r is in \ref data. */
      bool haskey( const std::string & key_r ) const
      { return _dataP && _dataP->find( key_r ) != _dataP->end(); }

      /** Whether \a key_r is in \ref data and value is not empty. */
      bool hasvalue( const std::string & key_r ) const
      {
	bool ret = false;
	if ( _dataP )
	{
	  const_iterator it = _dataP->find( key_r );
	  if ( it != _dataP->end() && ! it->second.empty() )
	  {
	    ret = true;
	  }
	}
	return ret;
      }

      /** Set the value for key (nonconst version always returns true).
       * Const version is allowed to set empty values or to add new ones only.
       */
      bool set( const std::string & key_r, boost::any val_r )
      { dataRef()[key_r] = std::move(val_r); return true; }
      /** \overload const version */
      bool set( const std::string & key_r, boost::any val_r ) const
      {
	bool ret = false;
	boost::any & val( dataRef()[key_r] );
	if ( val.empty() )
	{
	  val = std::move(val_r);
	  ret = true;
	}
	return ret;
      }

      /** Set an empty value for \a key_r (if possible). */
      bool reset( const std::string & key_r )
      { return set( key_r, boost::any() ); }
      /** \overload const version */
      bool reset( const std::string & key_r ) const
      { return set( key_r, boost::any() ); }

      /** Remove key from data.*/
      void erase( const std::string & key_r )
      { if ( _dataP ) _dataP->erase( key_r ); }

      /** Return the keys boost::any value or an empty value if key does not exist. */
      const boost::any & getvalue( const std::string & key_r ) const
      {
	if ( _dataP )
	{
	  const_iterator it = _dataP->find( key_r );
	  if ( it != _dataP->end() )
	  {
	    return it->second;
	  }
	}
	static const boost::any none;
	return none;
      }

      /** Pass back a <tt>const Tp &</tt> reference to \a key_r value.
       * \throws boost::bad_any_cast if key is not set or value is not of appropriate type
       * \code
       *   UserData data;
       *   std::string value( "defaultvalue" );
       *   try
       *   {
       *     value = data.get<std::string>( "mykey" );
       *   }
       *   catch ( const boost::bad_any_cast & )
       *   {
       *     // no "mykey" or not a std::sting
       *   }
       * \endcode
       */
      template <class Tp>
      const Tp & get( const std::string & key_r ) const
      { return boost::any_cast<const Tp &>( getvalue( key_r ) ); }

      /** Pass back a \a Tp copy of \a key_r value.
       * \throws boost::bad_any_cast if key is not set or value is not of appropriate type
       * \code
       *   UserData data;
       *   std::string value = data.get<std::string>( "mykey", "defaultvalue" );
       * \endcode
       */
      template <class Tp>
      Tp get( const std::string & key_r, const Tp & default_r ) const
      { Tp ret( default_r ); get( key_r, ret ); return ret; }

      /** If the value for \a key_r is of the same type as \a ret_r, pass it back in \a ret_r and return \c true;.
       * \code
       *   UserData data;
       *   std::string value( "defaultvalue" );
       *   if ( ! data.get<std::string>( "mykey", value )
       *   {
       *      // no "mykey" or not a std::sting
       *   }
       * \endcode
       */
      template <class Tp>
      bool get( const std::string & key_r, Tp & ret_r ) const
      {
	bool ret = false;
	if ( _dataP )
	{
	  const_iterator it = _dataP->find( key_r );
	  if ( it != _dataP->end() )
	  {
	    auto ptr = boost::any_cast<const Tp>(&it->second);
	    if ( ptr )
	    {
	      ret_r = *ptr;
	      ret = true;
	    }
	  }
	}
	return ret;
      }

    private:
      DataType & dataRef() const
      { if ( ! _dataP ) _dataP.reset( new DataType ); return *_dataP; }

    private:
      ContentType _type;
      mutable shared_ptr<DataType> _dataP;
    };

    /** \relates UserData Stream output */
    inline std::ostream & operator<<( std::ostream & str, const UserData & obj )
    { return str << "UserData(" << obj.type() << ":" << obj.size() << ")";}

  } // namespace callback
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_USERDATA_H
