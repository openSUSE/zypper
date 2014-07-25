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
    ///////////////////////////////////////////////////////////////////
    class UserData
    {
      typedef std::map<std::string,boost::any> DataType;
    public:
      /** Default ctor */
      UserData()
      {}

    public:
      /** Get type */
      const ContentType & type() const
      { return _type; }

      /** Set type */
      void type( const ContentType & type_r )
      { _type = type_r; }

    public:
      /**  Validate object in a boolean context: has data */
      explicit operator bool() const
      { return ! ( _dataP == nullptr || _dataP->empty() ); }

    private:
      ContentType		_type;
      shared_ptr<DataType>	_dataP;
    };

    /** \relates UserData Stream output */
    inline std::ostream & operator<<( std::ostream & str, const UserData & obj )
    { return str << "UserData(" << obj.type() << ")";}

  } // namespace callback
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_USERDATA_H
