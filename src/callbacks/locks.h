#ifndef LOCKS_ZYPPER_CALLBACKS_H
#define LOCKS_ZYPPER_CALLBACKS_H

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/PoolQuery.h"

#include "../prompt.h"
#include "../utils.h"
#include "../output/prompt.h"

namespace zypp {

  struct LocksSaveReportReceiver : public zypp::callback::ReceiveReport<zypp::SavingLocksReport>
  {
    virtual Action conflict( const PoolQuery& query, ConflictState state )
    {
      if (state == SAME_RESULTS)
         Zypper::instance()->out().error("The following query locks the same packages as the lock which you want remove:");
      else
         Zypper::instance()->out().error(_("The following query locks some of the objects you want to unlock:"));

      query.serialize(std::cout);

      return read_bool_answer(PROMPT_YN_REMOVE_LOCK ,
        "Do you want remove this lock?", true)? DELETE : IGNORE;
    }
  };

  struct CleanLocksReportReceiver : public zypp::callback::ReceiveReport<zypp::CleanEmptyLocksReport>
  {
    virtual Action execute( const PoolQuery& query )
    {
      Zypper::instance()->out().error(_("The following query does not lock anything:"));

      query.serialize(std::cout);

      return read_bool_answer(PROMPT_YN_REMOVE_LOCK ,
        "Do you want remove this lock?", true)? DELETE : IGNORE;
    }
  };

}


class LocksCallbacks {
  private:
    zypp::LocksSaveReportReceiver _saveReport;
    zypp::CleanLocksReportReceiver _cleanReport;

  public:
    LocksCallbacks()
    {
      _saveReport.connect();
      _cleanReport.connect();
    }
     
    ~LocksCallbacks()
    {
      _saveReport.disconnect();
      _cleanReport.disconnect();
    }
};

#endif
