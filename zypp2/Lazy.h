/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/LazyText.h
 *
*/
#ifndef ZYPP_LazyText_H
#define ZYPP_LazyText_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include <boost/function.hpp>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  template<class _T>
  class LazyLoadDataFunc
  {
    public:
    LazyLoadDataFunc()
    {}
    
    virtual ~LazyLoadDataFunc()
    {}
    
    virtual _T operator()() = 0;
  };
  
  class LazyLoadFromFileFunc : public LazyLoadDataFunc<std::string>
  {
    public:
    LazyLoadFromFileFunc( const zypp::filesystem::Pathname &path, long int start, long int offset);
    virtual std::string operator()();
        
    private:
    zypp::filesystem::Pathname _path;
    long int _start;
    long int _offset;
  };
  
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Lazy
  //
  
  template<class _T>
  class Lazy
  {
    template<class _A>
    friend std::ostream & operator<<( std::ostream & str, const Lazy<_A> & obj );
  public:
    
    Lazy( boost::function<_T ()> func )
    : _func(func)
    {}

    ~Lazy()
    {}

    _T value() const
    { return _func(); }
    
  private:
    boost::function< _T ()> _func;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates LazyText Stream output */
  template <class _T>
  inline std::ostream & operator<<( std::ostream & str, const Lazy<_T> & obj )
  { return str << obj.value(); }

  typedef Lazy<std::string> LazyText;
  
  inline Lazy<std::string> lazyTextFromFile( const zypp::filesystem::Pathname &path, long int start, long int offset )
  {
    Lazy<std::string> lazy(Lazy<std::string>( LazyLoadFromFileFunc( path, start, offset ) ));
    return lazy;
  }
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_LazyText_H
