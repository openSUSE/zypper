/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMFileListParser.cc
 *
*/

#include <sys/types.h>
#include <regex.h>

#include <zypp/parser/yum/YUMFileListParser.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlstring.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/parser/yum/schemanames.h>
#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/ZYppFactory.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace zypp
{
namespace parser
{
namespace yum
{

static boost::regex filenameRegex("^.*(/bin/|/sbin/|/lib/|/lib64/|/etc/|/usr/share/dict/words/|/usr/games/|/usr/share/magic\\.mime|/opt/gnome/games).*$");
static regex_t filenameRegexT;
static bool filenameRegexOk = false;

YUMFileListParser::YUMFileListParser(std::istream &is, const std::string& baseUrl, parser::ParserProgress::Ptr progress )
    : XMLNodeIterator<YUMFileListData_Ptr>(is, baseUrl,FILELISTSCHEMA, progress)
    , _zypp_architecture( getZYpp()->architecture() )
{
  fetchNext();
}

YUMFileListParser::YUMFileListParser()
    : _zypp_architecture( getZYpp()->architecture() )
{ }

YUMFileListParser::YUMFileListParser(YUMFileListData_Ptr& entry)
    : XMLNodeIterator<YUMFileListData_Ptr>(entry)
    , _zypp_architecture( getZYpp()->architecture() )
{ }



YUMFileListParser::~YUMFileListParser()
{}




// select for which elements process() will be called
bool
YUMFileListParser::isInterested(const xmlNodePtr nodePtr)
{
  bool result = (_helper.isElement(nodePtr)
                 && _helper.name(nodePtr) == "package");
  return result;
}


// do the actual processing
YUMFileListData_Ptr
YUMFileListParser::process(const xmlTextReaderPtr reader)
{
  xml_assert(reader);
  YUMFileListData_Ptr dataPtr = new YUMFileListData;
  xmlNodePtr dataNode = xmlTextReaderExpand(reader);
  xml_assert(dataNode);

  dataPtr->pkgId = _helper.attribute(dataNode,"pkgid");
  dataPtr->name = _helper.attribute(dataNode,"name");
  dataPtr->arch = _helper.attribute(dataNode,"arch");
  try
  {
    if (!Arch(dataPtr->arch).compatibleWith( _zypp_architecture ))
    {
      dataPtr = NULL;			// skip <package>, incompatible architecture
      return dataPtr;
    }
  }
  catch ( const Exception & excpt_r )
  {
    ZYPP_CAUGHT( excpt_r );
    DBG << "Skipping malformed " << dataPtr->arch << endl;
    dataPtr = NULL;
    return dataPtr;
  }

  for (xmlNodePtr child = dataNode->children;
       child != 0;
       child = child->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "version")
      {
        dataPtr->epoch = _helper.attribute(child,"epoch");
        dataPtr->ver = _helper.attribute(child,"ver");
        dataPtr->rel = _helper.attribute(child,"rel");
      }
      else if (name == "file")
      {
        string filename = _helper.content( child );
        if (!filenameRegexOk)
        {
          const char * filenameRegexPattern = "/(s?bin|lib(64)?|etc)/|^/usr/(games/|share/(dict/words|magic\\.mime)$)|^/opt/gnome/games/";
          regcomp (&filenameRegexT, filenameRegexPattern, REG_EXTENDED | REG_NOSUB);
          //MIL << "regcomp " << r;
          filenameRegexOk = true;
        }

        bool match;
        match = !regexec (&filenameRegexT, filename.c_str(), 0 /*nmatch*/, NULL /*pmatch*/, 0 /*flags*/);

        if (match)
          dataPtr->files.push_back( FileData( filename, _helper.attribute( child, "type" ) ) );
      }
      else
      {
        WAR << "YUM <filelists> contains the unknown element <" << name << "> "
        << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
  //std::cout << dataPtr->files.size() << std::endl;
  return dataPtr;
}


} // namespace yum
} // namespace parser
} // namespace zypp
