/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatternParser.cc
 *
*/

#include <zypp/parser/yum/YUMPatternParser.h>
#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMPatternParser::YUMPatternParser()
      { }
      
      YUMPatternParser::YUMPatternParser(YUMPatternData_Ptr& entry)
      : XMLNodeIterator<YUMPatternData_Ptr>(entry)
      { }
      
      
      YUMPatternParser::~YUMPatternParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMPatternParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "pattern";
      }
      
      // do the actual processing
      YUMPatternData_Ptr
      YUMPatternParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        YUMPatternData_Ptr dataPtr = new YUMPatternData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);
        
        for (xmlNodePtr child = dataNode->children;
             child && child != dataNode;
             child = child->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "patternid") {
                   dataPtr->patternId = _helper.content(child);
                 }
                 else if (name == "name") {
                   dataPtr->name.push_back
                     (MultiLang(_helper.attribute(child,"lang"),
                                _helper.content(child)));
                 }
                 else if (name == "default") {
                   dataPtr->default_ = _helper.content(child);
                 }
                 else if (name == "uservisible") {
                   dataPtr->userVisible = _helper.content(child);
                 }
                 else if (name == "description") {
                   dataPtr->description.push_back
                     (MultiLang(_helper.attribute(child,"lang"),
                                _helper.content(child)));
                 }
                 else if (name == "patternlist") {
                   parsePatternlist(dataPtr, child);
                 }
                 else if (name == "packagelist") {
                   parsePackageList(dataPtr, child);
                 }
                 else {
                   WAR << "YUM <pattern> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
        return dataPtr;
      } /* end process */
      
      void YUMPatternParser::parsePatternlist(YUMPatternData_Ptr dataPtr,
                                                xmlNodePtr node)
      {
        assert(dataPtr);
        assert(node);
        
        for (xmlNodePtr child = node->children;
             child != 0;
             child = child ->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "metapkg" || name == "patternreq") {
                   dataPtr->patternlist.push_back
                     (MetaPkg(_helper.attribute(child,"type"),
                              _helper.content(child)));
                 }
                 else {
                   WAR << "YUM <patternlist> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
      }
      
      
      void YUMPatternParser::parsePackageList(YUMPatternData_Ptr dataPtr,
                                                  xmlNodePtr node)
      {
        assert(dataPtr);
        assert(node);
        
        for (xmlNodePtr child = node->children;
             child != 0;
             child = child ->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "packagereq") {
                 dataPtr->packageList.push_back
                   (PackageReq(_helper.attribute(child,"type"),
                               _helper.attribute(child,"epoch"),
                               _helper.attribute(child,"ver"),
                               _helper.attribute(child,"rel"),
                               _helper.content(child)));
                 }
                 else {
                   WAR << "YUM <packagelist> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
      }
      
      
      
      YUMPatternParser::YUMPatternParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<YUMPatternData_Ptr>(is, baseUrl,PATTERNSCHEMA)
      { 
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
