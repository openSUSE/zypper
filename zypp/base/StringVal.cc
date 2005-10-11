/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/StringVal.cc
 *
*/
#include "zypp/base/StringVal.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::StringVal
    //	METHOD TYPE : Ctor
    //
    StringVal::StringVal()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::StringVal
    //	METHOD TYPE : Ctor
    //
    StringVal::StringVal( const std::string & rhs )
    : _value( rhs )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::StringVal
    //	METHOD TYPE : Ctor
    //
    StringVal::StringVal( const StringVal & rhs )
    : _value( rhs._value )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::~StringVal
    //	METHOD TYPE : Dtor
    //
    StringVal::~StringVal()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::operator=
    //	METHOD TYPE : const StringVal &
    //
    const StringVal & StringVal::operator=( const std::string & rhs )
    {
      _value = rhs;
      return *this;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StringVal::operator=
    //	METHOD TYPE : const StringVal &
    //
    const StringVal & StringVal::operator=( const StringVal & rhs )
    {
      _value = rhs._value;
      return *this;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
