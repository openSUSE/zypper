#include <iosfwd>
#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
// MyClass.h
///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /** Simple class using a RWCOW_pointer to implementation.
   *
   * MyClass maintains an int value, manipulated via get/set.
   * RWCOW_pointer provides the 'copy on write' functionality.
  */
  class MyClass
  {
  public:
    /** Implementation (public, but hidden in MyClass.cc) */
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
  //
  namespace debug
  {
    /** Forward decl. Implemented after MyClass::Impl is defined,
     * if you want to dynamic_cast TraceCAD<MyClass::Impl> back into
     * MyClass::Impl. Otherwise you could implement it here.
     */
    template<>
      void traceCAD( TraceCADBase::What what_r,
                     const TraceCAD<MyClass::Impl> & self_r,
                     const TraceCAD<MyClass::Impl> & rhs_r );
  }
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /** Implementation of MyClass providing the int value.
   *
   * To debug via TraceCAD, simply derive. Per default TraceCAD
   * drops loglines. In this example we overload traceCAD<Impl>,
   * to do a bit more stuff.
  */
  struct MyClass::Impl : public debug::TraceCAD<Impl>
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

  inline std::ostream & operator<<( std::ostream & str, const MyClass::Impl & obj )
  { return str << "MyClass::Impl[" << &obj << "] value: " << obj._value; }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // class MyClass
  ///////////////////////////////////////////////////////////////////

  MyClass::MyClass( int val )
  : _pimpl( new Impl( val ) )
  {
    // e.g _PING to indicate ctor is done.
    _pimpl->_PING();
  }

  /** Impl access via 'operator->() const' (readonly) */
  int MyClass::get() const
  { return _pimpl->_value; }

  /** Impl access via 'operator->()' (write, creates a copy iff shared) */
  void MyClass::set( int val )
  { _pimpl->_value = val; }

  ///////////////////////////////////////////////////////////////////
  //
  namespace debug
  {
    /** Performs all possible casts of self_r/rhs_r back into
     * MyClass::Impl.
     *
     * CTOR,DTOR: self_r can't be casted, because MyClass::Impl
     * is not yet constructed (TraceCAD is base class), or already
     * deleted. rhs_r is meaningless (==self_r).
     *
     * COPYCTOR: self_r can't be casted (not yet constructed).
     * rhs_r can be casted into MyClass::Impl. It's the object
     * we copy from.
     *
     * ASSIGN: self_r and rhs_r can be casted. If MyClass::Impl
     * defines an operator==, you have to alter the code to call
     * TraceCAD::operator=. Otherwise it won't be triggered.
     *
     * PING: self_r can be casted, rhs_r is meaningless (==self_r).
     * You have to alter MyClass::Impl to call '_PING()' to recieve
     * the trigger. The only purpose is to provide an easy way to
     * trigger some action, without much changes to the original code.
     * Call _PING there and perfrorm the action here, if possible.
    */
    template<>
      void traceCAD( TraceCADBase::What what_r,
                     const TraceCAD<MyClass::Impl> & self_r,
                     const TraceCAD<MyClass::Impl> & rhs_r )
      {
        static unsigned instanceCouter = 0;
        // lazy #define ;)
#define STATS "\t(total " << instanceCouter << ")"

        switch( what_r )
          {
          case TraceCADBase::CTOR:
            ++instanceCouter;
            SEC << self_r << what_r << STATS << std::endl;
            break;

          case TraceCADBase::DTOR:
            --instanceCouter;
            SEC << self_r << what_r << STATS << std::endl;
            break;

          case TraceCADBase::PING:
            SEC << dynamic_cast<const MyClass::Impl &>(self_r)
                << what_r << STATS << std::endl;
            break;

          case TraceCADBase::COPYCTOR:
            ++instanceCouter;
            SEC << self_r << what_r << "( "
                << dynamic_cast<const MyClass::Impl &>(rhs_r)
                << ")" << STATS << std::endl;
            break;

          case TraceCADBase::ASSIGN:
            SEC << dynamic_cast<const MyClass::Impl &>(self_r)
                << what_r << "( "
                << dynamic_cast<const MyClass::Impl &>(rhs_r)
                << ")" << STATS << std::endl;
            break;
          }
      }
  }
  //
  ///////////////////////////////////////////////////////////////////
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

  MyClass c; // MyClass::Impl CTOR
  MyClass d( c ); // shares Impl
  MyClass e( d ); // shares Impl

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will d.set( 4 )..." << endl;
  d.set( 4 ); // performs COW
  DBG << "done d.set( 4 )" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will e=d..." << endl;
  e = d; // shares Impl
  DBG << "done e=d" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  DBG << "will e.set( 1 )..." << endl;
  e.set( 1 ); // performs COW
  DBG << "done e.set( c )" << endl;

  MIL << "c.get" << c.get() << endl;
  MIL << "d.get" << d.get() << endl;
  MIL << "e.get" << e.get() << endl;

  INT << "===[END]============================================" << endl;
  return 0;
  // Tree times MyClass::Impl DTOR
}
