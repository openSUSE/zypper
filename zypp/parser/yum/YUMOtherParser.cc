/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMOtherParser.cc
 *
*/



#include <zypp/parser/yum/YUMOtherParser.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlstring.h>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {


      YUMOtherParser::YUMOtherParser(istream &is, const string& baseUrl)
      : XMLNodeIterator<YUMOtherData_Ptr>(is, baseUrl,OTHERSCHEMA)
      {
        fetchNext();
      }
      
      YUMOtherParser::YUMOtherParser()
      { }
      
      YUMOtherParser::YUMOtherParser(YUMOtherData_Ptr& entry)
      : XMLNodeIterator<YUMOtherData_Ptr>(entry)
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
      YUMOtherData_Ptr
      YUMOtherParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        YUMOtherData_Ptr dataPtr = new YUMOtherData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
      
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
                 else if (name == "changelog") {
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
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
