#include <iostream>

#include <boost/signal.hpp>
#include <boost/bind.hpp>

#include <zypp/base/LogTools.h>
#include <zypp/base/Easy.h>

using std::endl;
using std::cout;
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace boost
{
  template<class Tp>
      std::ostream & operator<<( std::ostream & str, const signal<Tp> & obj )
  {
    return str << "Connected slots: " << obj.num_slots();
  }

  namespace signals
  {
    std::ostream & operator<<( std::ostream & str, const connection & obj )
    {
      return str << "Connection: "
	  << ( obj.connected() ? '*' : '_' )
	  << ( obj.blocked()   ? 'B' : '_' )
	  ;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace zypp;

using boost::signal;
using boost::signals::connection;
using boost::signals::trackable;

struct HelloWorld
{
  HelloWorld() {++i;}
  HelloWorld(const HelloWorld &) {++i;}
  ~HelloWorld() { --i;}

  void operator()(unsigned) const
  {
    USR << "Hello, World! " << i << std::endl;
  }

  static int i;
};

int HelloWorld::i = 0;

struct M
{
  void ping() const
  {
    static unsigned i = 0;
    ++i;
    MIL << i << " -> " << _sigA << endl;
    _sigA( i );
  }

  typedef signal<void(unsigned)> SigA;

  SigA & siga() const { return _sigA; }

  mutable SigA _sigA;
};

struct X : public trackable
{
  X() {_s=++s;}
  X( const X & ) {_s=++s;}
  X& operator=( const X & ) { return *this; }
  ~X() {_s=-_s;}
  static int s;
  int _s;

  void pong( unsigned i ) const
  {
    DBG << _s << ' ' << i << endl;
  }
};

int X::s;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, const char * argv[] )
{
  --argc; ++argv; // skip arg 0

  M m;
  m.ping();
  X xx;
  m.siga().connect( boost::bind( &X::pong, &xx, _1 ) );
  m.ping();

  {
    X x;
    m.siga().connect( boost::bind( &X::pong, &x, _1 ) );
    m.ping();

    X y;
    m.siga().connect( boost::bind( &X::pong, &y, _1 ) );
    m.ping();

  }

  m.ping();

  ///////////////////////////////////////////

  INT << "---STOP" << endl;
  return 0;
}

