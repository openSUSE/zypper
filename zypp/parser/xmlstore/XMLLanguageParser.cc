/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLLanguageParser.cc
 *
*/

#include <zypp/parser/xmlstore/XMLLanguageParser.h>
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

      XMLLanguageParser::XMLLanguageParser()
      { }
      
      XMLLanguageParser::XMLLanguageParser(XMLLanguageData_Ptr& entry)
      : XMLNodeIterator<XMLLanguageData_Ptr>(entry)
      { }
      
      
      XMLLanguageParser::~XMLLanguageParser()
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLLanguageParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "language";
      }
      
      // do the actual processing
      XMLLanguageData_Ptr
      XMLLanguageParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        XMLLanguageData_Ptr dataPtr = new XMLLanguageData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
        
        parseResObjectCommonData( dataPtr, dataNode);
        parseDependencies( dataPtr, dataNode);
        
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            
            if (name == "summary")
            {
              dataPtr->summary.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "description") {
              dataPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
          }
        }
        return dataPtr;
      } /* end process */
      
      
      XMLLanguageParser::XMLLanguageParser(istream &is, const string &baseUrl)
      : XMLNodeIterator<XMLLanguageData_Ptr>(is, baseUrl,LANGUAGESCHEMA)
      { 
        fetchNext();
      }
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
