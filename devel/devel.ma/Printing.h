#ifndef MA_PRINTING_H
#define MA_PRINTING_H

#include <iostream>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/PtrTypes.h"
#include <zypp/base/String.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

///////////////////////////////////////////////////////////////////

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
  struct PPrint : public std::unary_function<_Tp, bool>
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
    std::for_each( c.begin(), c.end(),
                   Print<typename _Container::value_type>() );
  }

template<class _Container>
  void pprint( const _Container & c )
  {
    std::for_each( c.begin(), c.end(),
                   PPrint<typename _Container::value_type>() );
  }

///////////////////////////////////////////////////////////////////
#endif // MA_PRINTING_H
