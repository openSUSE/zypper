#include <iostream>
#include <sstream>

#include "Out.h"
#include "Table.h"

Out::~Out()
{

}

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

