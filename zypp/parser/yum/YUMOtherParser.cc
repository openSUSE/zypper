/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

File:       YUMOtherParser.cc

Author:     Michael Radziej <mir@suse.de>
Maintainer: Michael Radziej <mir@suse.de>

Purpose:    Parses other.xml files in a YUM repository

/-*/


#include <YUMOtherParser.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlstring.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <schemanames.h>

using namespace std;
namespace zypp { namespace parser { namespace YUM {


YUMOtherParser::YUMOtherParser(istream &is, const string& baseUrl)
: XMLNodeIterator<YUMOtherDataPtr>(is, baseUrl,OTHERSCHEMA)
{
  fetchNext();
}

YUMOtherParser::YUMOtherParser()
{ }

YUMOtherParser::YUMOtherParser(YUMOtherDataPtr& entry)
: XMLNodeIterator<YUMOtherDataPtr>(entry)
{ }



YUMOtherParser::~YUMOtherParser()
{
}




// select for which elements process() will be called
bool 
YUMOtherParser::isInterested(const xmlNodePtr nodePtr)
{
  bool result = (_helper.isElement(nodePtr)
                 && _helper.name(nodePtr) == "package");
  return result;
}


// do the actual processing
YUMOtherDataPtr
YUMOtherParser::process(const xmlTextReaderPtr reader)
{
  assert(reader);
  YUMOtherDataPtr dataPtr = new YUMOtherData;
  xmlNodePtr dataNode = xmlTextReaderExpand(reader);
  assert(dataNode);

  dataPtr->pkgId = _helper.attribute(dataNode,"pkgid");
  dataPtr->name = _helper.attribute(dataNode,"name");
  dataPtr->arch = _helper.attribute(dataNode,"arch");

  for (xmlNodePtr child = dataNode->children;
       child != 0;
       child = child->next) {
         if (_helper.isElement(child)) {
           string name = _helper.name(child);
           if (name == "version") {
             dataPtr->epoch = _helper.attribute(child,"epoch");
             dataPtr->ver = _helper.attribute(child,"ver");
             dataPtr->rel = _helper.attribute(child,"rel");
           }
           else if (name == "file") {
             dataPtr->changelog.push_back
               (ChangelogEntry(_helper.attribute(child,"author"),
                               _helper.attribute(child,"date"),
                               _helper.content(child)));
           }
           else {
             WAR << "YUM <otherdata> contains the unknown element <" << name << "> "
               << _helper.positionInfo(child) << ", skipping" << endl;
           }
         }
       }
  return dataPtr;
}

}}}
