/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/StringVal.h
 *
*/
#ifndef ZYPP_BASE_STRINGVAL_H
#define ZYPP_BASE_STRINGVAL_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : StringVal
    //
    /** */
    class StringVal
    {
    public:
      /** */
      operator const std::string &() const
      { return _value; }
      /** */
      const std::string & asString() const
      { return _value; }
    protected:
      /** */
      StringVal();
      /** */
      explicit
      StringVal( const std::string & rhs );
      /** */
      StringVal( const StringVal & rhs );
      /** */
      ~StringVal();
      /** */
      const StringVal & operator=( const std::string & rhs );
      /** */
      const StringVal & operator=( const StringVal & rhs );
    private:
      std::string _value;
    };
    ///////////////////////////////////////////////////////////////////

    inline std::ostream & operator<<( std::ostream & str, const StringVal & obj )
    { return str << static_cast<const std::string &>(obj); }

    ///////////////////////////////////////////////////////////////////

    inline bool operator==( const StringVal & lhs, const StringVal & rhs )
    { return lhs.asString() == rhs.asString(); }

    inline bool operator==( const StringVal & lhs, const std::string & rhs )
    { return lhs.asString() == rhs; }

    inline bool operator==( const std::string & lhs, const StringVal & rhs )
    { return lhs == rhs.asString(); }


    inline bool operator!=( const StringVal & lhs, const StringVal & rhs )
    { return !( lhs == rhs ); }

    inline bool operator!=( const StringVal & lhs, const std::string & rhs )
    { return !( lhs == rhs ); }

    inline bool operator!=( const std::string & lhs, const StringVal & rhs )
    { return !( lhs == rhs ); }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRINGVAL_H
