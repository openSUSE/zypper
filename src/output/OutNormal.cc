#include <iostream>

#include "OutNormal.h"

using std::cout;
using std::cerr;
using std::endl;

OutNormal::~OutNormal()
{
  
}

bool OutNormal::mine(Type type)
{
  // Type::TYPE_NORMAL is mine
  if (type & Out::TYPE_NORMAL)
    return true;
  return false;
}

bool OutNormal::infoWarningFilter(Verbosity verbosity, Type mask)
{
  if (!mine(mask))
    return true;
  if (this->verbosity() < verbosity)
    return true;
  return false;
}

void OutNormal::info(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  cout << msg << endl;
}

void OutNormal::warning(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  info(msg, verbosity, mask);
}

void OutNormal::error(const std::string & problem_desc, const std::string & hint)
{
  cerr << problem_desc;
  if (!hint.empty())
    cerr << endl << hint;
  cerr << endl;
}

void OutNormal::error(const zypp::Exception & e,
                      const std::string & problem_desc,
                      const std::string & hint)
{
  
}


void OutNormal::progressStart(){}
void OutNormal::progress(){}
void OutNormal::progressEnd(){}

void OutNormal::dwnldProgressStart(){}
void OutNormal::dwnldProgress(){}
void OutNormal::dwnldProgressEnd(){}
