#include <iostream>
#include <sstream>

#include "Out.h"
#include "Table.h"

#include "Zypper.h"

////////////////////////////////////////////////////////////////////////////////
//	class Out
////////////////////////////////////////////////////////////////////////////////

Out::~Out()
{}

bool Out::progressFilter()
{
  if (this->verbosity() < Out::NORMAL)
      return true;
  return false;
}

std::string Out::zyppExceptionReport(const zypp::Exception & e)
{
  std::ostringstream s;
  // The Exception history is a stack! So zypp::Exception::asUserHistory() prints:
  //   This is bad!          <- Exception::asUserString()
  //   History:            -+
  //    - top level error   |<- Exception::historyAsString()
  //    - mid level error   |
  //    - first error      -+
  if (this->verbosity() > Out::NORMAL)
    s << e.asUserHistory();
  else
    s << e.asUserString();

  return s.str();
}

void Out::searchResult( const Table & table_r )
{
  std::cout << table_r;
}

////////////////////////////////////////////////////////////////////////////////
//	class Out::Error
////////////////////////////////////////////////////////////////////////////////

int Out::Error::report( Zypper & zypper_r ) const
{
  if ( ! ( _msg.empty() && _hint.empty() ) )
    zypper_r.out().error( _msg, _hint );
  if ( _exitcode != ZYPPER_EXIT_OK )	// ZYPPER_EXIT_OK indicates exitcode is already set.
    zypper_r.setExitCode( _exitcode );
  return zypper_r.exitCode();
}
