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
  virtual void progressEnd(const std::string & id,
                           const std::string & label,
                           bool error);

  // progress with download rate
  virtual void dwnldProgressStart(const zypp::Url & uri);
  virtual void dwnldProgress(const zypp::Url & uri,
                             int value = -1,
                             long rate = -1);
  virtual void dwnldProgressEnd(const zypp::Url & uri,
                                long rate = -1,
                                bool error = false);

  virtual void searchResult( const Table & table_r );

  virtual void prompt(PromptId id,
                      const std::string & prompt,
                      const PromptOptions & poptions,
                      const std::string & startdesc = "");

  virtual void promptHelp(const PromptOptions & poptions);

protected:
  virtual bool mine(Type type);

private:
  bool infoWarningFilter(Verbosity verbosity, Type mask);
  void writeProgressTag(const std::string & id,
                        const std::string & label,
                        int value, bool done, bool error = false);
};

#endif /*OUTXML_H_*/
