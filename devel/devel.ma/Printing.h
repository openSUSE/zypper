#ifndef MA_PRINTING_H
#define MA_PRINTING_H

#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include <zypp/base/Logger.h>

#include <zypp/base/String.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

using std::endl;

///////////////////////////////////////////////////////////////////

struct Print
{
  template<class Tp>
    bool operator()( const Tp & val_r ) const
    { USR << val_r << endl; return true; }
};

///////////////////////////////////////////////////////////////////

template<class Tp>
  struct PrintOn : public std::unary_function<Tp, bool>
  {
    bool operator()( const Tp & obj ) const
    {
      if ( _leadNL )
        _str << std::endl << _prfx << obj;
      else
        _str << _prfx << obj << std::endl;
      return true;
    }

    PrintOn( std::ostream & str, const std::string & prfx = std::string(), bool leadNL = false )
    : _str( str )
    , _prfx( prfx )
    , _leadNL( leadNL )
    {}

    std::ostream & _str;
    std::string _prfx;
    bool _leadNL;
  };

///////////////////////////////////////////////////////////////////
#endif // MA_PRINTING_H
