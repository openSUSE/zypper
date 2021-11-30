#ifndef OUTXML_H_
#define OUTXML_H_

#include "Out.h"

class OutXML : public Out
{
public:
  OutXML( Verbosity verbosity );
  ~OutXML() override;

public:
  void info( const std::string & msg, Verbosity verbosity, Type mask ) override;
  void warning( const std::string & msg, Verbosity verbosity, Type mask ) override;
  void error( const std::string & problem_desc, const std::string & hint ) override;
  void error( const Exception & e, const std::string & problem_desc, const std::string & hint ) override;

  // progress
  void progressStart( const std::string & id, const std::string & label, bool is_tick ) override;
  void progress( const std::string & id, const std::string & label, int value ) override;
  void progressEnd( const std::string & id, const std::string & label, const std::string & donetag, bool error ) override;

  // progress with download rate
  void dwnldProgressStart( const Url & uri ) override;
  void dwnldProgress( const Url & uri, int value, long rate ) override;
  void dwnldProgressEnd( const Url & uri, long rate, TriBool error ) override;

  void searchResult( const Table & table_r ) override;

  void prompt( PromptId id, const std::string & prompt, const PromptOptions & poptions, const std::string & startdesc ) override;

  void promptHelp( const PromptOptions & poptions ) override;

protected:
  bool mine( Type type ) override;

private:
  bool infoWarningFilter( Verbosity verbosity, Type mask );
  void writeProgressTag( const std::string & id, const std::string & label, int value, bool done, bool error = false );
};

#endif /*OUTXML_H_*/
