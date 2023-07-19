/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_UTILS_TEXT_H_
#define ZYPPER_UTILS_TEXT_H_

#include <zypp-tui/utils/text.h>

namespace mbs  {
  using ztui::mbs::MbsIterator;
  using ztui::mbs::MbsIteratorNoSGR;
  using ztui::mbs::MbsWriteWrapped;
  using ztui::mbs::MbToWc;
}

using ztui::mbs_write_wrapped;
using ztui::mbs_substr_by_width;
using ztui::mbs_width;

#endif //ZYPPER_UTILS_TEXT_H_
