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



using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {


      YUMFileListParser::YUMFileListParser(istream &is, const string& baseUrl)
      : XMLNodeIterator<YUMFileListData_Ptr>(is, baseUrl,FILELISTSCHEMA)
      {
        fetchNext();
      }
      
      YUMFileListParser::YUMFileListParser()
      { }
      
      YUMFileListParser::YUMFileListParser(YUMFileListData_Ptr& entry)
      : XMLNodeIterator<YUMFileListData_Ptr>(entry)
      { }
      
      
      
      YUMFileListParser::~YUMFileListParser()
      {
      }
      
      
      
      
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
                   dataPtr->files.push_back
                     (FileData(_helper.content(child),
                               _helper.attribute(child,"type")));
                 }
                 else {
                   WAR << "YUM <filelists> contains the unknown element <" << name << "> "
                     << _helper.positionInfo(child) << ", skipping" << endl;
                 }
               }
             }
        return dataPtr;
      }
  

    } // namespace yum
  } // namespace parser
} // namespace zypp
