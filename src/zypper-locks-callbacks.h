#ifndef LOCKS_ZYPPER_CALLBACKS_H
#define LOCKS_ZYPPER_CALLBACKS_H

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/PoolQuery.h"

#include "zypper-prompt.h"
#include "zypper-utils.h"
#include "output/prompt.h"

namespace zypp {

  struct LocksSaveReportReceiver : public zypp::callback::ReceiveReport<zypp::SavingLocksReport>
  {
    virtual Action conflict( const PoolQuery& query, ConflictState state )
    {
      //TODO localize
      if (state==SAME_RESULTS)
      {
         Zypper::instance()->out().error("this query have lock same results as lock which you want remove:");
      }
      else
      {
         Zypper::instance()->out().error("this query lock some of objects, which you want unlock:");
      }

      query.serialize(std::cout);

      return read_bool_answer(PROMPT_YN_REMOVE_LOCK ,
        "Do you want remove this lock?", true)? DELETE : IGNORE;
    }
  };

  struct CleanLocksReportReceiver : public zypp::callback::ReceiveReport<zypp::CleanEmptyLocksReport>
  {
    virtual Action execute( const PoolQuery& query )
    {
      Zypper::instance()->out().error("this query doesn't lock anything:");

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
