/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPERINFO_H_
#define ZYPPERINFO_H_

#include <zypp/PoolItem.h>
#include <zypp/ResKind.h>
#include <zypp/ui/Selectable.h>
#include <zypp/base/Flags.h>

#include "Zypper.h"
#include "utils/misc.h" // for ResKindSet; might make sense to move this elsewhere

enum class InfoBits {
  DefaultView     = 0,
  ShowProvides    = 1 << 0,
  ShowRequires    = 1 << 1,
  ShowConflicts   = 1 << 2,
  ShowObsoletes   = 1 << 3,
  ShowRecommends  = 1 << 4,
  ShowSupplements = 1 << 5,
  ShowSuggests    = 1 << 6
};
ZYPP_DECLARE_FLAGS( InfoFlags, InfoBits );


struct PrintInfoOptions {
  bool _matchSubstrings = false;
  std::set<zypp::ResKind> _kinds;
  InfoFlags _flags;
};

void printInfo(Zypper & zypper, const std::vector<std::string> &names_r, const PrintInfoOptions &options_r, bool & all_caps_exist );

#endif /*ZYPPERINFO_H_*/
