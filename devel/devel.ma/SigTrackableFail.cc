#include <iostream>
using std::endl;
using std::cout;

#include <boost/signal.hpp>
#include <boost/bind.hpp>
using boost::signal;
using boost::signals::connection;
using boost::signals::scoped_connection;
using boost::signals::trackable;

#define DBG std::cerr
#define MIL std::cerr
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

struct Sender
{
  void ping() const
  {
    static unsigned i = 0;
    ++i;
    MIL << "Sending " << i << " -> " << _sigA << endl;
    _sigA( i );
    _sigB();
  }

  typedef signal<void(unsigned)> SigA;
  typedef signal<void(void)>     SigB;

  SigA & siga() const { return _sigA; }
  SigB & sigb() const { return _sigB; }

  mutable SigA _sigA;
  mutable SigB _sigB;
};

struct Receiver : public trackable
{
  Receiver() {_s=++s;}
  Receiver( const Receiver & rhs ) : trackable( rhs ) {_s=++s;}
  Receiver& operator=( const Receiver & rhs ) {
    trackable::operator=( rhs );
    return *this;
  }
  ~Receiver() {_s=-_s;}
  static int s;
  int _s;

  void fpong()
  {
    dumpOn( DBG << "Receiver " << _s << " <- "  << 13 << " (" ) << ")" << endl;
  }

  void operator()( unsigned i )
  { pong( i ); }

  void pong( unsigned i )
  {
    dumpOn( DBG << "Receiver " << _s << " <- "  << i << " (" ) << ")" << endl;
  }

  std::ostream & dumpOn( std::ostream & str ) const
  {
    return str << "Receiver " << _s << " connected signals: " << _connected_signals().size();
  }
};

std::ostream & operator<<( std::ostream & str, const Receiver & obj )
{ return obj.dumpOn( str ); }

int Receiver::s;

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, const char * argv[] )
{
  --argc; ++argv; // skip arg 0

  Sender sender;
  sender.ping();

  Receiver rec;
  sender.siga().connect( boost::bind( &Receiver::pong, &rec, _1 ) );
  sender.ping();

  {
    Receiver recw;
    sender.siga().connect( boost::ref(recw) );
    sender.ping();

    Receiver recx;
    MIL << recx << endl;
    sender.siga().connect( boost::bind( &Receiver::pong, &recx, _1 ) );
    sender.sigb().connect( boost::bind( &Receiver::fpong, &recx ) );
    MIL << recx << endl;
    sender.ping();

    Receiver recy;
    connection cy( sender.siga().connect( boost::bind( &Receiver::pong, &recy, _1 ) ) );
    sender.ping();

    Receiver recz;
    scoped_connection cz( sender.siga().connect( boost::bind( &Receiver::pong, &recz, _1 ) ) );
    sender.ping();

    cy.disconnect();
  }

  sender.ping();
  return 0;
}

