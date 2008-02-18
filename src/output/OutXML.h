#ifndef OUTXML_H_
#define OUTXML_H_

#include "Out.h"

class OutXML : public Out
{
public:
  OutXML(Verbosity verbosity = NORMAL);
  virtual ~OutXML();

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
  void dwnldProgressStart();
  void dwnldProgress();
  void dwnldProgressEnd();

protected:
  virtual bool mine(Type type);

private:
  bool infoWarningFilter(Verbosity verbosity, Type mask);
  std::string reportZyppException(const zypp::Exception & e);
};

#endif /*OUTXML_H_*/
