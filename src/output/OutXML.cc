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
                              int value, bool done, bool error)
{
  cout << "<progress ";
  cout << " id=\"" << id << "\"";
  cout << " name=\"" << label << "\"";
  if (done)
    cout << " done=\"" << error << "\"";
  else if (value >= 0)
    cout << " value=\"" << value << "\"";
  cout << "/>" << endl;
}

void OutXML::progressStart(const string & id,
                           const string & label,
                           bool has_range)
{
  if (progressFilter())
    return;

  //! \todo there is a bug in progress data which returns has_range false incorrectly
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

void OutXML::progressEnd(const string & id, const string& label, bool error)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, 100, true, error);
}

// progress with download rate
void OutXML::dwnldProgressStart(const zypp::Url & uri)
{
  cout << "<not-implemented what=\"dwnlod-progress-start\">" << endl;
}

void OutXML::dwnldProgress(const zypp::Url & uri,
                           int value,
                           int rate)
{
  cout << "<not-implemented what=\"dwnlod-progress\">" << endl;
}

void OutXML::dwnldProgressEnd(const zypp::Url & uri, bool error)
{
  cout << "<not-implemented what=\"dwnlod-progress-end\">" << endl;
}

void OutXML::prompt(PromptId id,
                    const string & prompt,
                    const string & answer_hint) // hint ignored for now, maybe an enumeration will be here in the future
{
  cout << "<prompt id=\"" << id << "\">" << prompt << "</prompt>" << endl;
}
