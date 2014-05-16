#include <iostream>
#include <sstream>
#include <vector>

#include <zypp/base/String.h>
#include <zypp/base/String.h>

#include "OutXML.h"
#include "utils/misc.h"
#include "Table.h"

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;
using std::vector;

OutXML::OutXML(Verbosity verbosity_r) : Out(TYPE_XML, verbosity_r)
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

bool OutXML::infoWarningFilter(Verbosity verbosity_r, Type mask)
{
  if (!mine(mask))
    return true;
  if (verbosity() < verbosity_r)
    return true;
  return false;
}

/*
string xmlEncode(const string & s)
{
  string result;
  result.reserve(s.size()*2);
  char buff[7];

  for(string::const_iterator it = s.begin(); it != s.end(); ++it)
  {
    switch(*it)
    {
    case '&':
    case '<':
    case '>':
    case '"':
    {
      snprintf(buff, 7, "&#%d;", (unsigned)*it);
      result.append(buff);
      break;
    }
    default:
      result.append(1, *it);
    }
  }

  return result;
}
*/

void OutXML::info(const string & msg, Verbosity verbosity_r, Type mask)
{
  if (infoWarningFilter(verbosity_r, mask))
    return;

  cout << "<message type=\"info\">" << xml::escape(msg)
       << "</message>" << endl;
}

void OutXML::warning(const string & msg, Verbosity verbosity_r, Type mask)
{
  if (infoWarningFilter(verbosity_r, mask))
    return;

  cout << "<message type=\"warning\">" << xml::escape(msg)
       << "</message>" << endl;
}

void OutXML::error(const string & problem_desc, const string & hint)
{
  cout << "<message type=\"error\">" << xml::escape(problem_desc)
       << "</message>" << endl;
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

  cout << "<message type=\"error\">" << xml::escape(s.str())
       << "</message>" << endl;
}

void OutXML::writeProgressTag(const string & id, const string & label,
                              int value, bool done, bool error)
{
  cout << "<progress";
  cout << " id=\"" << xml::escape(id) << "\"";
  cout << " name=\"" << xml::escape(label) << "\"";
  if (done)
    cout << " done=\"" << error << "\"";
  // print value only if it is known (percentage progress)
  // missing value means 'is-alive' notification
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

void OutXML::dwnldProgressStart(const zypp::Url & uri)
{
  cout << "<download"
    << " url=\"" << xml::escape(uri.asString()) << "\""
    << " percent=\"-1\""
    << " rate=\"-1\""
    << "/>" << endl;
}

void OutXML::dwnldProgress(const zypp::Url & uri,
                           int value,
                           long rate)
{
  cout << "<download"
    << " url=\"" << xml::escape(uri.asString()) << "\""
    << " percent=\"" << value << "\""
    << " rate=\"" << rate << "\""
    << "/>" << endl;
}

void OutXML::dwnldProgressEnd(const zypp::Url & uri, long rate, bool error)
{
  cout << "<download"
    << " url=\"" << xml::escape(uri.asString()) << "\""
    << " rate=\"" << rate << "\""
    << " done=\"" << error << "\""
    << "/>" << endl;
}

void OutXML::searchResult( const Table & table_r )
{
  cout << "<search-result version=\"0.0\">" << endl;
  cout << "<solvable-list>" << endl;

  const Table::container & rows( table_r.rows() );
  if ( ! rows.empty() )
  {
    //
    // *** CAUTION: It's a mess, but must match the header list defined
    //              in FillSearchTableSolvable ctor (search.cc)
    // We derive the XML tag from the header, applying some translation
    // hence and there.
    std::vector<std::string> header;
    {
      const TableHeader & theader( table_r.header() );
      for_( it, theader.columns().begin(), theader.columns().end() )
      {
	if ( *it == "S" )
	  header.push_back( "status" );
	else if ( *it == "Type" )
	  header.push_back( "kind" );
	else if ( *it == "Version" )
	  header.push_back( "edition" );
	else
	  header.push_back( zypp::str::toLower( *it ) );
      }
    }

    for_( it, rows.begin(), rows.end() )
    {
      cout << "<solvable";
      const TableRow::container & cols( it->columns() );
      unsigned cidx = 0;
      for_( cit, cols.begin(), cols.end() )
      {
	cout << ' ' << (cidx < header.size() ? header[cidx] : "?" ) << "=\"";
	if ( cidx == 0 )
	{
	  if ( *cit == "i" )
	    cout << "installed\"";
	  else if ( *cit == "v" )
	    cout << "other-version\"";
	  else
	    cout << "not-installed\"";
	}
	else
	{
	  cout << xml::escape(*cit) << '"';
	}
	++cidx;
      }
      cout << "/>" << endl;
    }
  }
    //Out::searchResult( table_r );

  cout << "</solvable-list>" << endl;
  cout << "</search-result>" << endl;
}

void OutXML::prompt(PromptId id,
                    const string & prompt,
                    const PromptOptions & poptions,
                    const string & startdesc)
{
  cout << "<prompt id=\"" << id << "\">" << endl;
  if (!startdesc.empty())
    cout << "<description>" << xml::escape(startdesc) << "</description>" << endl;
  cout << "<text>" << xml::escape(prompt) << "</text>" << endl;

  unsigned int i = 0;
  for (PromptOptions::StrVector::const_iterator it = poptions.options().begin();
       it != poptions.options().end(); ++it, ++i)
  {
    if (poptions.isDisabled(i))
      continue;
    string option = *it;
    cout << "<option";
    if (poptions.defaultOpt() == i)
      cout << " default=\"1\"";
    cout << " value=\"" << xml::escape(option) << "\"";
    cout << " desc=\"" << xml::escape(poptions.optionHelp(i)) << "\"";
    cout << "/>" << endl;
  }
  cout << "</prompt>" << endl;
}

void OutXML::promptHelp(const PromptOptions & poptions)
{
  // nothing to do here
}
