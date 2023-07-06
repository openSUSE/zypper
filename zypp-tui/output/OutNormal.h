/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/

#ifndef OUTNORMAL_H_
#define OUTNORMAL_H_

#include <zypp-tui/output/Out.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace ztui {

class OutNormal : public Out
{
public:
  OutNormal(Verbosity verbosity = NORMAL);
  ~OutNormal() override;

public:
  void info( const std::string & msg, Verbosity verbosity, Type mask ) override;
  void infoLine( const TermLine & msg, Verbosity verbosity, Type mask ) override;
  void warning( const std::string & msg, Verbosity verbosity, Type mask ) override;
  void error( const std::string & problem_desc, const std::string & hint ) override;
  void error( const zypp::Exception & e, const std::string & problem_desc, const std::string & hint ) override;

  // progress
  void progressStart( const std::string & id, const std::string & label, bool is_tick ) override;
  void progress( const std::string & id, const std::string & label, int value ) override;
  void progressEnd( const std::string & id, const std::string & label, const std::string & donetag, bool error) override;

  // progress with download rate
  void dwnldProgressStart( const zypp::Url & uri ) override;
  void dwnldProgress( const zypp::Url & uri, int value, long rate ) override;
  void dwnldProgressEnd( const zypp::Url & uri, long rate, zypp::TriBool error ) override;

  void prompt( PromptId id, const std::string & prompt, const PromptOptions & poptions, const std::string & startdesc ) override;

  void promptHelp( const PromptOptions & poptions ) override;

  void setUseColors( bool value ) override
  { _use_colors = value; }

protected:
  bool mine( Type type ) override;

  /* Return current terminal width or 'unsigned(-1)' when failed */
  unsigned termwidth() const override;

private:
  bool infoWarningFilter(Verbosity verbosity, Type mask);
  void fixupProgressNL(); //< Make sure we're at BOL even if a ProgressBar is active
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

}

#endif /*OUTNORMAL_H_*/
