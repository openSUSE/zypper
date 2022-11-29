#include "ShutdownLock_p.h"

#include <zypp/base/LogTools.h>
#include <zypp/ExternalProgram.h>
#include <iostream>

zypp::ShutdownLock::ShutdownLock(const std::string &reason)
{
  try {
    MIL << "Try to acquire an inhibitor lock..." << endl;
    std::string whyStr = str::form("--why=%s", reason.c_str());

    const char* argv[] =
    {
      "/usr/bin/systemd-inhibit",
      "--what=sleep:shutdown:idle",
      "--who=zypp",
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
    _prog->kill(15);
    _prog->close();
  }
}
