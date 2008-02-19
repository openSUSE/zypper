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

void OutXML::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  ostringstream s;

  // problem
  s << problem_desc << endl;
  // cause
  s << zyppExceptionReport(e) << endl;
  // hint
  if (!hint.empty())
    s << hint << endl;

  cout << "<message type=\"error\">" << s.str() << "</message>" << endl;
}

void OutXML::writeProgressTag(const string & id, const string & label,
                              int value, bool done)
{
  cout << "<progress ";
  cout << " id=\"" << id << "\"";
  cout << " name=\"" << label << "\"";
  if (value >= 0)
    cout << " value=\"" << value << "\"";
  cout << " done=\"" << done << "\"";
  cout << "/>" << endl;
}

void OutXML::progressStart(const string & id,
                           const string & label,
                           bool has_range)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, has_range ? 0 : -1, false);
}

void OutXML::progress(const string & id,
                      const string& label,
                      int value)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, value, false);
}

void OutXML::progressEnd(const string & id, const string& label)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, 100, true);
}

// progress with download rate
void OutXML::dwnldProgressStart(const std::string & id,
                                const std::string & label)
{
  cout << "<not-implemented what=\"dwnlod-progress-start\">" << endl;
}

void OutXML::dwnldProgress(const std::string & id,
                           const std::string & label,
                           int value,
                           int rate)
{
  cout << "<not-implemented what=\"dwnlod-progress\">" << endl;
}

void OutXML::dwnldProgressEnd(const std::string & id,
                              const std::string & label)
{
  cout << "<not-implemented what=\"dwnlod-progress-end\">" << endl;
}