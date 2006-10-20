/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLProductParser.cc
 *
*/

#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include "zypp/parser/xmlstore/XMLProductParser.h"
#include "zypp/parser/xmlstore/XMLResObjectParser.h"
#include "zypp/parser/LibXMLHelper.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/xmlstore/schemanames.h"

using namespace std;
namespace zypp {
  namespace parser {
    namespace xmlstore {

      XMLProductParser::~XMLProductParser()
      { }
      
      XMLProductParser::XMLProductParser(istream &is, const string& baseUrl)
        : XMLNodeIterator<XMLProductData_Ptr>(is, baseUrl ,PRODUCTSCHEMA)
      {
        fetchNext();
      }
      
      XMLProductParser::XMLProductParser()
      { }
      
      XMLProductParser::XMLProductParser(XMLProductData_Ptr& entry)
      : XMLNodeIterator<XMLProductData_Ptr>(entry)
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLProductParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "product";
      }
      
      // do the actual processing
      XMLProductData_Ptr
      XMLProductParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        XMLProductData_Ptr productPtr = new XMLProductData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
        productPtr->type = _helper.attribute(dataNode,"type");
              
        parseResObjectCommonData( productPtr, dataNode);
        parseDependencies( productPtr, dataNode);
        
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "vendor") {
	      productPtr->vendor = _helper.content(child);
            }
            else if (name == "release-notes-url") {
              productPtr->releasenotesurl = _helper.content(child);
            }
            else if (name == "distribution-name") {
              productPtr->dist_name = _helper.content(child);
            }
            else if (name == "distribution-edition") {
              productPtr->dist_version = _helper.content(child);
            }
            else if (name == "shortname") {
              productPtr->short_name.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "product-flags") {
              parseProductFlags( productPtr, child);
            }
            
	    else if (name == "update-urls") {
              parseUpdateUrls( productPtr, child);
	    }
          }
        }
        return productPtr;
      } /* end process */
      
      void
      XMLProductParser::parseProductFlags( XMLProductData_Ptr productPtr, xmlNodePtr node)
      {
        for (xmlNodePtr child = node->children; child && child != node; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "product-flag")
            {
              productPtr->flags.push_back(_helper.content(child));
            }
          }
        }
      }
      
      void
      XMLProductParser::parseUpdateUrls( XMLProductData_Ptr productPtr, xmlNodePtr node)
      {
        for (xmlNodePtr child = node->children; child && child != node; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "update-url")
            {
              productPtr->update_urls.push_back(_helper.content(child));
            }
          }
        }
      }
    } // namespace yum
  } // namespace parser
} // namespace zypp
