/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_PS_INCLUDED
#define ZYPPER_COMMANDS_PS_INCLUDED

#include "commands/basecommand.h"
#include "utils/flags/zyppflags.h"

class PSCommand : public ZypperBaseCommand
{
public:
  PSCommand();

  // ZypperBaseCommand interface
  std::string description() const override;

protected:
  std::vector<zypp::ZyppFlags::CommandOption> cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp, const std::vector<std::string> &positionalArgs) override;

  void printServiceNamesOnly();
  bool tableWithFilesEnabled() const		{ return _shortness < 1; }
  bool tableWithNonServiceProcsEnabled() const	{ return _shortness < 2; }
  bool printServiceNamesOnlyEnabled() const	{ return _shortness >= 3; }
  bool debugEnabled() const {return (!_debugFile.empty());}

private:
  int _shortness = 0;
  std::string _format;
  std::string _debugFile;
};


#endif
