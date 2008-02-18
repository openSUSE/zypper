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
