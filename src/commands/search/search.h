/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SEARCH_SEARCH_H_INCLUDED
#define ZYPPER_COMMANDS_SEARCH_SEARCH_H_INCLUDED

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"


#include <zypp/sat/SolvAttr.h>
#include <boost/optional.hpp>

class SearchCmd : public ZypperBaseCommand
{
public:

  enum MatchMode {
    Default,
    Substrings,
    Words,
    Exact
  };

  enum CmdMode {
    Search,
    RugPatchSearch
  };

  SearchCmd ( std::vector<std::string> &&commandAliases_r, CmdMode cmdMode_r = CmdMode::Search );

  void setMode(const MatchMode &mode_r );
  void addRequestedDependency ( const zypp::sat::SolvAttr &dep_r );

private:
  CmdMode _cmdMode;
  MatchMode _mode = MatchMode::Default;
  bool _forceNameAttr = false;
  bool _searchFileList = false;
  bool _searchDesc = false;
  bool _caseSensitive = false;
  bool _details = false;
  bool _verbose = false;
  std::set<zypp::sat::SolvAttr> _requestedDeps;
  boost::optional<zypp::sat::SolvAttr> _requestedReverseSearch;

  std::set<ResKind> _requestedTypes;

  //careful when adding new optionsets, only enable them for the right command mode
  NotInstalledOnlyOptionSet _notInstalledOpts;
  SortResultOptionSet _sortOpts { *this };
  InitReposOptionSet _initReposOpts;

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r) override;

  // ZypperBaseCommand interface
public:
  std::string summary() const override;
  std::vector<std::string> synopsis() const override;
  std::string description() const override;
  std::string help() override;
};



#endif
