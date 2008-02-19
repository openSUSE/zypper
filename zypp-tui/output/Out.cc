#include <sstream>

#include "Out.h"

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
  if (e.historySize())
  {
    if (this->verbosity() > Out::NORMAL)
    {
      // print the whole history
      s << e.historyAsString();
      // this exception
      s << " - " << e.asUserString();
    }
    else
      // print the root cause only
      s << *(--e.historyEnd());
  }
  else
    s << e.asUserString();

  return s.str();
}
