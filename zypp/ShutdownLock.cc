#include "ShutdownLock_p.h"

#include <zypp/ExternalProgram.h>
#include <iostream>

zypp::ShutdownLock::ShutdownLock(const std::string &reason)
{
  try {

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
    _prog->kill();
  }
}
