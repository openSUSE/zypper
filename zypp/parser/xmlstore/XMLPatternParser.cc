/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLPatternParser.cc
 *
*/

#include <zypp/parser/xmlstore/XMLPatternParser.h>
#include <zypp/parser/xmlstore/XMLResObjectParser.h>

#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/xmlstore/schemanames.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace xmlstore {

      XMLPatternParser::XMLPatternParser()
      { }
      
      XMLPatternParser::XMLPatternParser(XMLPatternData_Ptr& entry)
      : XMLNodeIterator<XMLPatternData_Ptr>(entry)
      { }
      
      
      XMLPatternParser::~XMLPatternParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLPatternParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "pattern";
      }
      
      // do the actual processing
      XMLPatternData_Ptr
      XMLPatternParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        XMLPatternData_Ptr dataPtr = new XMLPatternData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
        
        parseResObjectCommonData( dataPtr, dataNode);
        parseDependencies( dataPtr, dataNode);
        
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            
            if (name == "default") {
              dataPtr->default_ = _helper.content(child);
            }
            else if (name == "uservisible") {
              dataPtr->userVisible = (_helper.content(child) == "false") ? false : true;
            }
            else if (name == "category") {
              dataPtr->category.setText(_helper.content(child));
            }
            else if (name == "icon") {
              dataPtr->icon = _helper.content(child);
            }
            else if (name == "script") {
              dataPtr->script = _helper.content(child);
            }
          }
        }
        return dataPtr;
      } /* end process */
      
      
      XMLPatternParser::XMLPatternParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<XMLPatternData_Ptr>(is, baseUrl,PATTERNSCHEMA)
      { 
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
