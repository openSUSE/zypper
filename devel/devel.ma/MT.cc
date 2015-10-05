//#include <thread>
#include <vector>
#include <boost/thread.hpp>
#include <zypp/base/Functional.h>

#include <zypp/base/Easy.h>
#include <zypp/base/Measure.h>
#include <zypp/base/Exception.h>
#include <zypp/base/String.h>

#include <zypp/base/LogTools.h>
#include <zypp/base/LogControl.h>

#include "zypp/ExternalProgram.h"

using std::endl;
using zypp::debug::Measure;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /** Run a number of tasks using \c threadCount threads.
   *
   * \code
   * std::vector<function<void()>> tasks;
   * for ( int i = 0; i < 100; ++i )
   * {
   *   tasks.push_back( [i]()
   *   {
   *     MIL << '[' << i << "]" << endl;
   *   });
   * }
   * runTasks( tasks, 10 );
   * \endcode
   */
  template <class TFunction>
  void runTasks( const std::vector<TFunction>& tasks, size_t threadCount = 1 )
  {
    if ( threadCount )
    {
      boost::thread_group group;
      const size_t taskCount = (tasks.size() / threadCount) + 1;
      for ( size_t start = 0; start < tasks.size(); start += taskCount )
      {
	group.create_thread( [&tasks, start, taskCount]()
	{
	  const size_t end = std::min( tasks.size(), start + taskCount );
	  for ( size_t i = start; i < end; ++i )
	    tasks[i]();
	});
      }
      group.join_all();
    }
    else
    {
      for_( f, tasks.begin(), tasks.end() )
	(*f)();
    }
  }
}
///////////////////////////////////////////////////////////////////
using namespace zypp;
int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ///////////////////////////////////////////////////////////////////

  std::vector<function<void()>> tasks;
  for ( int i = 0; i < 100; ++i )
  {
    tasks.push_back( [i]()
    {
      MIL << '[' << i << "]" << endl;
    });
  }
  {
    Measure x( "THREAD" );
    runTasks( tasks, 100 );
  }


  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const zypp::Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}

