#ifndef OUTNORMAL_H_
#define OUTNORMAL_H_

#include "Out.h"

class OutNormal : public Out
{
public:
  OutNormal(Verbosity verbosity = NORMAL) : Out(verbosity) {}
  virtual ~OutNormal();

public:
  virtual void info(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL);
  virtual void warning(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL);
  virtual void error(const std::string & problem_desc, const std::string & hint = "");
  virtual void error(const zypp::Exception & e,
             const std::string & problem_desc,
             const std::string & hint = "");

  // progress
  virtual void progressStart(const std::string & id,
                             const std::string & label,
                             bool is_tick = false);
  virtual void progress(const std::string & id,
                        const std::string & label,
                        int value = -1);
  virtual void progressEnd(const std::string & id, const std::string & label);

  // progress with download rate
  virtual void dwnldProgressStart(const zypp::Url & uri);
  virtual void dwnldProgress(const zypp::Url & uri,
                             int value = -1,
                             int rate = -1);
  virtual void dwnldProgressEnd(const zypp::Url & uri);
  
  virtual void prompt(PromptId id,
                      const std::string & prompt,
                      const std::string & answer_hint);

protected:
  virtual bool mine(Type type);

private:
  bool infoWarningFilter(Verbosity verbosity, Type mask);
};

#endif /*OUTNORMAL_H_*/
