#include <iosfwd>
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
// MyClass.h
///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  class MyClass
  ///////////////////////////////////////////////////////////////////
  {
  public:
    /** Implementation */
    struct Impl;

  public:
    MyClass( int val = 0 );

    int get() const;

    void set( int val );

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////
// MyClass.cc
///////////////////////////////////////////////////////////////////
#include <zypp/base/Debug.h>

using std::endl;

namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  struct MyClass::Impl : public debug::TraceCAD<Impl>
  ///////////////////////////////////////////////////////////////////
  {
    Impl( int val = 0 )
    : _value( val )
    {}

    int _value;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // class MyClass
  ///////////////////////////////////////////////////////////////////

  MyClass::MyClass( int val )
  : _pimpl( new Impl( val ) )
  {}

  int MyClass::get() const
  { return _pimpl->_value; }

  void MyClass::set( int val )
  { _pimpl->_value = val; }

}
///////////////////////////////////////////////////////////////////

/******************************************************************
**
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
**
**      DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  using zypp::MyClass;

  MyClass c;
  MyClass d( c );
  MyClass e( d );

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will d.set( 4 )..." << endl;
  d.set( 4 );
  DBG << "done d.set( 4 )" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will d.set( 5 )..." << endl;
  d.set( 5 );
  DBG << "done d.set( 5 )" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will c.set( 1 )..." << endl;
  c.set( 5 );
  DBG << "done c.set( c )" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}

#if 0

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace debug
  { /////////////////////////////////////////////////////////////////

    template<>
      void watchCAD( WatchCADBase::What what_r,
                     const WatchCAD<Dependencies::Impl> & self_r,
                     const WatchCAD<Dependencies::Impl> & rhs_r )
      {
        switch( what_r )
          {
          case WatchCADBase::CTOR:
          case WatchCADBase::DTOR:
            SEC << self_r << what_r << std::endl;
            break;
          case WatchCADBase::COPYCTOR:
          case WatchCADBase::ASSIGN:
            SEC << self_r << what_r << "( "
            << dynamic_cast<const Dependencies::Impl &>(rhs_r)
            << ")" << std::endl;
            break;
          }
      }

    /////////////////////////////////////////////////////////////////
  } // namespace debug
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif
