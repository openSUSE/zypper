/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_UTILS_OFFERING_H
#define ZYPPER_UTILS_OFFERING_H

#include <memory>

/// \brief Offer scoped handles to let requesters indicate their demand.
/// Store the \ref ScopedDemand handle returned by \ref demand as long
/// as you need it. The request ends if the handle goes out of scope, if
/// you assign a different \ref ScopedDemand handle to it or explicitly
/// call \ref handle.reset()
struct Offering
{
  struct _Content {};
  using ScopedDemand = std::shared_ptr<const _Content>;

  ScopedDemand demand()
  {
    ScopedDemand ret { _demand.lock() };
    if ( not ret ) {
      _demand = ret = std::make_shared<const _Content>();
    }
    return ret;
  }

  bool isDemanded() const
  { return not _demand.expired(); }

  unsigned demandCount() const
  { return _demand.use_count(); }

private:
  std::weak_ptr<const _Content> _demand;
};

#endif // ZYPPER_UTILS_OFFERING_H
