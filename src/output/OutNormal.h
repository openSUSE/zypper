/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef OUTNORMAL_H_
#define OUTNORMAL_H_

#include "Out.h"
#include <termios.h>
#include <sys/ioctl.h>

class OutNormal : public Out
{
public:
  OutNormal(Verbosity verbosity = NORMAL);
  virtual ~OutNormal();

public:
  /**
   * Prints \a msg to the standard output and appends a newline.
   *
   *  \see Out::info()
   */
  virtual void info(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL);

  /**
   * Prints info message optionally trunkated or expanded.
   */
  virtual void infoLine(const TermLine & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL);

  /**
   * Prints \a msg prepended with <tt>"Warning: "</tt> to the standard output
   * and appends a newline.
   *
   * \see Out::warning
   */
  virtual void warning(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL);

  /**
   *
   */
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

  virtual void prompt(PromptId id,
                      const std::string & prompt,
                      const PromptOptions & poptions,
                      const std::string & startdesc = "");

  virtual void promptHelp(const PromptOptions & poptions);

  void setUseColors(bool value)
  { _use_colors = value; }

protected:
  virtual bool mine(Type type);

  /* Return current terminal width or 'unsigned(-1)' when failed */
  virtual unsigned termwidth() const;

private:
  bool infoWarningFilter(Verbosity verbosity, Type mask);
  void displayProgress(const std::string & s, int percent);
  void displayTick(const std::string & s);

  bool _use_colors;
  bool _isatty;
  /* Newline flag. false if the last output did not end with new line character
   * (like in a self-overwriting progress line), false otherwise. */
  bool _newline;
  /* True if the last output line was longer than the terminal width */
  bool _oneup;
};

#endif /*OUTNORMAL_H_*/
