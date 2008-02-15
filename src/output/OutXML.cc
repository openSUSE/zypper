#include <iostream>

#include "OutXML.h"

using std::cout;
using std::cerr;
using std::endl;


OutXML::OutXML(Verbosity verbosity) : Out(verbosity)
{
  cout << "<?xml version='1.0'?>" << endl;
  cout << "<stream>" << endl;
}

OutXML::~OutXML()
{
  cout << "</stream>" << endl;
}

bool OutXML::mine(Type type)
{
  // Type::TYPE_NORMAL is mine
  if (type & Out::TYPE_XML)
    return true;
  return false;
}

bool OutXML::infoWarningFilter(Verbosity verbosity, Type mask)
{
  if (!mine(mask))
    return true;
  if (this->verbosity() < verbosity)
    return true;
  return false;
}

void OutXML::info(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  //! \todo xml escape the msg
  cout << "<message type=\"info\">" << msg << "</message>" << endl;
}

void OutXML::warning(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  cout << "<message type=\"warning\">" << msg << "</message>" << endl;
}

void OutXML::error(const std::string & problem_desc, const std::string & hint)
{
  cerr << "<message type=\"error\">" << problem_desc << "</message>" << endl;
  //! \todo hint
}

void OutXML::error(const zypp::Exception & e,
                      const std::string & problem_desc,
                      const std::string & hint)
{
  
}


void OutXML::progressStart(){}
void OutXML::progress(){}
void OutXML::progressEnd(){}

void OutXML::dwnldProgressStart(){}
void OutXML::dwnldProgress(){}
void OutXML::dwnldProgressEnd(){}
