#include <iostream>
#include <sstream>

#include "OutXML.h"

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;

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

void OutXML::info(const string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  //! \todo xml escape the msg
  cout << "<message type=\"info\">" << msg << "</message>" << endl;
}

void OutXML::warning(const string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  cout << "<message type=\"warning\">" << msg << "</message>" << endl;
}

void OutXML::error(const string & problem_desc, const string & hint)
{
  cout << "<message type=\"error\">" << problem_desc << "</message>" << endl;
  //! \todo hint
}

string OutXML::reportZyppException(const zypp::Exception & e)
{
  ostringstream s;
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


void OutXML::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  ostringstream s;

  // problem
  s << problem_desc << endl;
  // cause
  s << reportZyppException(e) << endl;
  // hint
  if (!hint.empty())
    s << hint << endl;

  cout << "<message type=\"error\">" << s.str() << "</message>" << endl;
}

void OutXML::progressStart(const string & id,
                           const string & label,
                           bool is_tick)
{
  if (progressFilter())
    return;

  cout << "<progress type=\"" << (is_tick ? "tick" : "percentage") << "\"";
  cout << " id=\"" << id << "\"";
  cout << " name=\"" << label << "\">";
  cout << endl;
}

void OutXML::progress(const string & id,
                      const string& label,
                      int value)
{
  if (progressFilter())
    return;

  if (value)
    cout << "<tick value=\"" << value << "\"/>" << endl;
  else
    cout << "<tick/>" << endl;
}

void OutXML::progressEnd(const string & id, const string& label)
{
  if (progressFilter())
    return;

  cout << "</progress>" << endl;
}


void OutXML::dwnldProgressStart(){}
void OutXML::dwnldProgress(){}
void OutXML::dwnldProgressEnd(){}
