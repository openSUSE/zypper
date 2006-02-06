/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/store/xml/XMLSourceCacheParser.cc
 *
*/

#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include <cassert>
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
//#include <zypp/parser/yum/schemanames.h>

#include <zypp/target/store/xml/XMLSourceCacheParser.h>

using namespace std;

namespace zypp {
    namespace storage {

      XMLSourceCacheParser::XMLSourceCacheParser()
      { }
      
      XMLSourceCacheParser::XMLSourceCacheParser(SourceData_Ptr& entry)
      : zypp::parser::XMLNodeIterator<SourceData_Ptr>(entry)
      { }
      
      
      XMLSourceCacheParser::~XMLSourceCacheParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLSourceCacheParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "source-cache";
      }
      
      // do the actual processing
      SourceData_Ptr
      XMLSourceCacheParser::process(const xmlTextReaderPtr reader)
      {
        assert(reader);
        SourceData_Ptr dataPtr = new PersistentStorage::SourceData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        assert(dataNode);
        
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "enabled")
            {
              dataPtr->enabled = (_helper.content(child) == "true") ? true : false;
            }
            else if (name == "auto-refresh")
            {
              dataPtr->autorefresh = (_helper.content(child) == "true") ? true : false;
            }
            else if (name == "type")
            {
              dataPtr->type = _helper.content(child);
            }
            else if (name == "product-dir")
            {
              dataPtr->product_dir = _helper.content(child);
            }
            else if (name == "alias")
            {
              dataPtr->alias = _helper.content(child);
            }
            else if (name == "url")
            {
              dataPtr->url = _helper.content(child);
            }
            else
            {
              WAR << "SourceCache entry contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        return dataPtr;
      } /* end process */
      
      
      XMLSourceCacheParser::XMLSourceCacheParser(istream &is, const string &baseUrl)
      : zypp::parser::XMLNodeIterator<SourceData_Ptr>(is, baseUrl, "")
      { 
        fetchNext();
      }
      
  } // namespace storage
} // namespace zypp
