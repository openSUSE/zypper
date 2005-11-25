/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMRepomdParser.cc
 *
*/

#include <zypp/parser/yum/YUMRepomdParser.h>
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

      YUMRepomdParser::~YUMRepomdParser()
      { }
      
      YUMRepomdParser::YUMRepomdParser()
      { }
      
      YUMRepomdParser::YUMRepomdParser(YUMRepomdData_Ptr& entry)
      : XMLNodeIterator<YUMRepomdData_Ptr>(entry)
      { }
      
      
      // select for which elements process() will be called
      bool 
      YUMRepomdParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "data";
      }
      
      // do the actual processing
      YUMRepomdData_Ptr
      YUMRepomdParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        YUMRepomdData_Ptr repoPtr = new YUMRepomdData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);
        repoPtr->type = _helper.attribute(dataNode,"type");
        
        for (xmlNodePtr child = dataNode->children; 
             child && child != dataNode;
             child = child->next) {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "location") {
              repoPtr->location = _helper.attribute(child,"href");
            }
            else if (name == "checksum") {
              repoPtr->checksumType = _helper.attribute(child,"type");
              repoPtr->checksum = _helper.content(child);
            }
            else if (name == "timestamp") {
              repoPtr->timestamp = _helper.content(child);
            }
            else if (name == "open-checksum") {
              repoPtr->openChecksumType = _helper.attribute(child, "type");
              repoPtr->openChecksum = _helper.content(child);
            }
            else {
              WAR << "YUM <data> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        return repoPtr;
      } /* end process */
      
        
      YUMRepomdParser::YUMRepomdParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<YUMRepomdData_Ptr>(is, baseUrl,REPOMDSCHEMA)
      {
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
