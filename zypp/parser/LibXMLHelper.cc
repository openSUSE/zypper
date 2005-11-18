/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/LibXMLHelper.cc
 *
*/
#include <LibXMLHelper.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <cassert>
#include <sstream>

namespace zypp {

  namespace parser {

    using namespace std;
    
    LibXMLHelper::LibXMLHelper()
    { }
    
    LibXMLHelper::~LibXMLHelper()
    { }
    
    std::string LibXMLHelper::attribute(xmlNode * nodePtr, 
                                        const string &name, 
                                        const string &defaultValue) const
    {
      assert(nodePtr);
      xmlChar *xmlRes = xmlGetProp(nodePtr, BAD_CAST(name.c_str()));
      if (xmlRes == 0)
        return defaultValue;
      else {
        string res((const char *)xmlRes);
        xmlFree(xmlRes);
        return res;
      }
    }
    
    
    std::string LibXMLHelper::content(xmlNode * nodePtr) const
    {
      assert(nodePtr);
      xmlChar *xmlRes = xmlNodeGetContent(nodePtr);
      if (xmlRes == 0)
        return string();
      else {
        string res((const char*) xmlRes);
        xmlFree(xmlRes);
        return res;
      }
    }
    
    std::string LibXMLHelper::name(const xmlNode * nodePtr) const
    {
      assert(nodePtr);
      return string((const char*) nodePtr->name);
    }
    
    
    bool LibXMLHelper::isElement(const xmlNode * nodePtr) const
    {
      return nodePtr->type == XML_ELEMENT_NODE;
    }
    
    std::string LibXMLHelper::positionInfo(const xmlNode * nodePtr) const
    {
      stringstream strm;
      strm << nodePtr->line;
      return string("at line ") + strm.str();
    }
  }
}
