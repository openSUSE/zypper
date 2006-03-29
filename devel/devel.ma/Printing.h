#ifndef MA_PRINTING_H
#define MA_PRINTING_H

#include <iostream>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/PtrTypes.h"
#include <zypp/base/String.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

///////////////////////////////////////////////////////////////////

template<class _Tp>
  struct PrintOn : public std::unary_function<_Tp, bool>
  {
    bool operator()( const _Tp & obj )
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

template<class _Tp>
  struct Print : public std::unary_function<_Tp, bool>
  {
    bool operator()( const _Tp & obj )
    {
      USR << obj << std::endl;
      return true;
    }
  };

template<class _Tp>
  struct PrintPtr : public std::unary_function<_Tp, bool>
  {
    bool operator()( const _Tp & obj )
    {
      if ( obj )
        USR << *obj << std::endl;
      else
        USR << "(NULL)" << std::endl;
      return true;
    }
  };

template<class _Container>
  void print( const _Container & c )
  {
    INT << c.size() << " " << __PRETTY_FUNCTION__ << std::endl;
    std::for_each( c.begin(), c.end(),
                   Print<typename _Container::value_type>() );
  }

template<class _Container>
  void printPtr( const _Container & c )
  {
    INT << c.size() << " " << __PRETTY_FUNCTION__ << std::endl;
    std::for_each( c.begin(), c.end(),
                   PrintPtr<typename _Container::value_type>() );
  }

template<class _Container>
  void printMK( const _Container & c )
  {
    for ( typename _Container::const_iterator it = c.begin(); it != c.end(); ++it )
      {
        USR << it->first << std::endl;
      }
  }
template<class _Container>
  void printMV( const _Container & c )
  {
    for ( typename _Container::const_iterator it = c.begin(); it != c.end(); ++it )
      {
        USR << it->second << std::endl;
      }
  }

template<class _Container>
  void printMKV( const _Container & c )
  {
    for ( typename _Container::const_iterator it = c.begin(); it != c.end(); ++it )
      {
        USR << it->first << '\t' << it->second << std::endl;
      }
  }

///////////////////////////////////////////////////////////////////
#endif // MA_PRINTING_H
