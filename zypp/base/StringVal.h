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
      operator const std::string &() const
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

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_STRINGVAL_H
