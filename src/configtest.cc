/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>

#include <zypp/base/LogTools.h>

#include "Zypper.h"
#include "configtest.h"

using std::cout;
using std::endl;

///////////////////////////////////////////////////////////////////
/// ConfigtestOptions
///////////////////////////////////////////////////////////////////

inline std::ostream & operator<<( std::ostream & str, const ConfigtestOptions & obj )
{ return str << "ConfigtestOptions"; }

///////////////////////////////////////////////////////////////////
namespace
{
  ///////////////////////////////////////////////////////////////////
  /// \class Configtest
  /// \brief Implementation of configtest commands.
  ///////////////////////////////////////////////////////////////////
  class Configtest
  {
    typedef ConfigtestOptions Options;
  public:
    Configtest( Zypper & zypper_r )
    : _zypper( zypper_r )
     , _options( _zypper.commandOptionsAs<ConfigtestOptions>() )
    { MIL << "Configtest " << _options << endl; }

  public:
    void run();

  private:
    Zypper & _zypper;				//< my Zypper
    shared_ptr<ConfigtestOptions> _options;	//< my Options
  };
  ///////////////////////////////////////////////////////////////////

  void Configtest::run()
  {
  }

} // namespace
///////////////////////////////////////////////////////////////////

int configtest( Zypper & zypper_r )
{
  try
  {
    Configtest( zypper_r ).run();
  }
  catch ( const Out::Error & error_r )
  {
    return error_r.report( zypper_r );
  }
  return zypper_r.exitCode();
}
