#include <iostream>

#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>
namespace boost
{
  namespace detail
  {
    inline std::ostream & operator<<( std::ostream & str, const thread_data_base & obj )
    { return str << &obj; }
  }
}

#include "zypp/ByteCount.h"
#include "zypp/Pathname.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"

using std::endl;
using namespace zypp;

#undef MIL
#define MIL MilSync( L_BASEFILE, __FUNCTION__, __LINE__ )._str

#ifdef _REENTRANT
#warning _REENTRANT
#else
#warning NOT_REENTRANT
#endif

template <class Derived>
struct ClassLevelLockable
{
  typedef boost::recursive_mutex Lockable;

  typedef boost::lock_guard<Lockable> Lock;
  struct Lock
  {
    Lock( const Derived & obj )
    {}
    ~Lock
  };

  Lockable _mutex;
};

template <class Derived>
struct ObjectLevelLockable
{
  typedef boost::recursive_mutex Lockable;
  typedef boost::lock_guard<Lockable> Lock;

};


struct MilSync
{
  MilSync( const char * file_r, const char * func_r, const int line_r )
    : _guard( _mutex )
    , _str( zypp::base::logger::getStream( ZYPP_BASE_LOGGER_LOGGROUP, zypp::base::logger::E_MIL, file_r, func_r, line_r ) )
  {}
  typedef boost::recursive_mutex Lockable;
  static Lockable             _mutex;
  boost::lock_guard<Lockable> _guard;
  std::ostream & _str;
};
MilSync::Lockable MilSync::_mutex;

struct ThreadExcl
{
  ThreadExcl()
  {
    MIL << "+TE" << boost::this_thread::get_id() << endl;
    boost::this_thread::sleep(  boost::posix_time::seconds(1) );
  }

  ~ThreadExcl()
  {
    MIL << "-TE" << boost::this_thread::get_id() << endl;
  }
};

void t_exit()
{
  MIL << "---" << boost::this_thread::get_id() << endl;
}

void t_main()
{
  MIL << "+++" << boost::this_thread::get_id() << " " << boost::this_thread::interruption_enabled() << endl;
  boost::this_thread::at_thread_exit( t_exit );
  ThreadExcl a;
  while( true )
    boost::this_thread::sleep(  boost::posix_time::seconds(1) );
}

int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "log.restrict" );
  INT << "===[START]==========================================" << endl;

  MIL << "M+++" << boost::this_thread::get_id() << endl;
  boost::thread_group mthreads;

  mthreads.create_thread( t_main );
  mthreads.create_thread( t_main );
  mthreads.create_thread( t_main );
  mthreads.create_thread( t_main );
  mthreads.create_thread( t_main );

  MIL << "M???" << boost::this_thread::get_id() << endl;
  //boost::this_thread::sleep(  boost::posix_time::seconds(10) );
  mthreads.interrupt_all();
  mthreads.join_all();
  MIL << "M---" << boost::this_thread::get_id() << endl;

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
