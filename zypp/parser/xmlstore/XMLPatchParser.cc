/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLPatchParser.cc
*
*/

#include <zypp/parser/xmlstore/XMLPatchParser.h>

#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/xmlstore/schemanames.h>

using namespace std;

namespace zypp {
  namespace parser {
    namespace xmlstore {

      XMLPatchParser::~XMLPatchParser()
      { }
      
      XMLPatchParser::XMLPatchParser(istream &is, const string& baseUrl)
        : XMLNodeIterator<XMLPatchData_Ptr>(is, baseUrl ,PATCHSCHEMA)
      {
        fetchNext();
      }
      
      XMLPatchParser::XMLPatchParser()
      { }
      
      XMLPatchParser::XMLPatchParser(XMLPatchData_Ptr& entry)
      : XMLNodeIterator<XMLPatchData_Ptr>(entry)
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLPatchParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "patch";
      }
      
      // do the actual processing
      XMLPatchData_Ptr
      XMLPatchParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        XMLPatchData_Ptr patchPtr = new XMLPatchData();
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
        
        // default values for optional entries
        patchPtr->rebootNeeded = false;
        patchPtr->packageManager = false;
      
        parseResObjectCommonData( patchPtr, dataNode);
        parseDependencies( patchPtr, dataNode);
        
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            
            if (name == "summary") {
              patchPtr->summary.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "description") {
              patchPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "id") {
              patchPtr->patchId = _helper.content(child);
            }
            else if (name == "timestamp") {
              patchPtr->timestamp = _helper.content(child);
            }
            else if (name == "category") {
              patchPtr->category = _helper.content(child);
            }
            else if (name == "reboot-needed") {
              patchPtr->rebootNeeded = (_helper.content(child) == "true") ? true : false;
            }
            else if (name == "affects-package-manager") {
              patchPtr->packageManager = (_helper.content(child) == "true") ? true : false;
            }
            else if (name == "interactive") {
              patchPtr->interactive = (_helper.content(child) == "true") ? true : false;
            }
            else if (name == "atoms") {
              parseAtomsNode(patchPtr, child);
            }
          }
        }
        return patchPtr;
      } /* end process */
      
      
      void 
      XMLPatchParser::parseAtomsNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        xml_assert(formatNode);
        for (xmlNodePtr child = formatNode->children; child != 0; child = child ->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            XXX << "parseAtomsNode(" << name << ")" << endl;
            if (name == "atom")
            {
                parseAtomNode (dataPtr, child);
            }
            else if (name == "script")
            {
                parseScriptNode (dataPtr, child);
            }
            else if (name == "message")
            {
                parseMessageNode (dataPtr, child);
            }
          }
        }
      } 
    
      void
      XMLPatchParser::parseAtomNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        XMLPatchAtomData_Ptr atom(new XMLPatchAtomData);
        // inject dependencies and other stuff
        parseResObjectCommonData( atom, formatNode);
        parseDependencies( atom, formatNode);
              
        dataPtr->atoms.push_back(atom);
      }
      
      void
      XMLPatchParser::parseScriptNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        XMLPatchScriptData_Ptr script(new XMLPatchScriptData);
        
        parseResObjectCommonData( script, formatNode);
        parseDependencies( script, formatNode);
        
        for (xmlNodePtr child = formatNode->children;  child != 0; child = child ->next)
        { 
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "do") {
              script->do_script = _helper.content(child);
            }
            else if (name == "undo")
            {
              script->undo_script = _helper.content(child);
            }
          }
        }
        dataPtr->atoms.push_back(script);
      }
      
      void
      XMLPatchParser::parseMessageNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        XMLPatchMessageData_Ptr message(new XMLPatchMessageData);
        
        parseResObjectCommonData( message, formatNode);
        parseDependencies( message, formatNode);
        
        for (xmlNodePtr child = formatNode->children;  child != 0; child = child ->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "text") {
              message->text.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
          }
        }        
        dataPtr->atoms.push_back(message);
      }

    } // namespace yum
  } // namespace parser
} // namespace zypp
