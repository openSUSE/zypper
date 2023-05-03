#include "ShutdownLock_p.h"

#include <zypp-core/base/LogTools.h>
#include <zypp-core/ExternalProgram.h>
#include <iostream>
#include <signal.h>

zypp::ShutdownLock::ShutdownLock(const std::string &who, const std::string &reason)
{
  try {
    MIL << "Try to acquire an inhibitor lock..." << endl;
    std::string whoStr = str::form("--who=%s", who.c_str());
    std::string whyStr = str::form("--why=%s", reason.c_str());

    const char* argv[] =
    {
      "/usr/bin/systemd-inhibit",
      "--what=sleep:shutdown:idle",
      whoStr.c_str(),
      "--mode=block",
      whyStr.c_str(),
      "/usr/bin/cat",
      NULL
    };
    _prog = shared_ptr<ExternalProgramWithSeperatePgid>( new ExternalProgramWithSeperatePgid( argv, ExternalProgram::Discard_Stderr ) );
  } catch (...) {
  }
}

zypp::ShutdownLock::~ShutdownLock()
{
  if (_prog) {
    MIL << "Terminate inhibitor lock: pid " << _prog->getpid() << endl;
    _prog->kill( SIGTERM );
    if ( !_prog->waitForExit( 10 * 1000 ) ) {
      // do a real kill if the app does not close in 10 seconds
      WAR << "systemd-inhibit did not respond to SIGTERM, killing it" << std::endl;
      _prog->kill();
    } else {
      _prog->close();
    }
  }
}
