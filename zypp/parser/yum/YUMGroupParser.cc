/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMGroupParser.cc
 *
*/

#include <YUMGroupParser.h>
#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMGroupParser::YUMGroupParser()
      { }
      
      YUMGroupParser::YUMGroupParser(YUMGroupData_Ptr& entry)
      : XMLNodeIterator<YUMGroupData_Ptr>(entry)
      { }
      
      
      YUMGroupParser::~YUMGroupParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMGroupParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "group";
      }
      
      // do the actual processing
      YUMGroupData_Ptr
      YUMGroupParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        YUMGroupData_Ptr dataPtr = new YUMGroupData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);
        
        for (xmlNodePtr child = dataNode->children;
             child && child != dataNode;
             child = child->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "groupid") {
                   dataPtr->groupId = _helper.content(child);
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
                 else if (name == "grouplist") {
                   parseGrouplist(dataPtr, child);
                 }
                 else if (name == "packagelist") {
                   parsePackageList(dataPtr, child);
                 }
                 else {
                   WAR << "YUM <group> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
        return dataPtr;
      } /* end process */
      
      void YUMGroupParser::parseGrouplist(YUMGroupData_Ptr dataPtr,
                                                xmlNodePtr node)
      {
        assert(dataPtr);
        assert(node);
        
        for (xmlNodePtr child = node->children;
             child != 0;
             child = child ->next) {
               if (_helper.isElement(child)) {
                 string name = _helper.name(child);
                 if (name == "metapkg" || name == "groupreq") {
                   dataPtr->grouplist.push_back
                     (MetaPkg(_helper.attribute(child,"type"),
                              _helper.content(child)));
                 }
                 else {
                   WAR << "YUM <grouplist> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
      }
      
      
      void YUMGroupParser::parsePackageList(YUMGroupData_Ptr dataPtr,
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
      
      
      
      YUMGroupParser::YUMGroupParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<YUMGroupData_Ptr>(is, baseUrl,GROUPSCHEMA)
      { 
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
