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
        : XMLNodeIterator<XMLPatchData_Ptr>(is, baseUrl /*,PATCHSCHEMA*/)
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
        patchPtr->timestamp = _helper.attribute(dataNode,"timestamp");
        patchPtr->patchId = _helper.attribute(dataNode,"patchid");
        patchPtr->engine = _helper.attribute(dataNode,"engine");
      
        // default values for optional entries
        patchPtr->rebootNeeded = false;
        patchPtr->packageManager = false;
      
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child)) {
            string name = _helper.name(child);
            if (name == "name") {
            patchPtr->name = _helper.content(child);
            }
            else if (name == "summary") {
              patchPtr->summary.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "description") {
              patchPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "version") {
              patchPtr->epoch = _helper.attribute(child,"epoch");
              patchPtr->ver = _helper.attribute(child,"ver");
              patchPtr->rel = _helper.attribute(child,"rel");
            }
            else if (name == "provides") {
              parseDependencyEntries(& patchPtr->provides, child);
            }
            else if (name == "conflicts") {
              parseDependencyEntries(& patchPtr->conflicts, child);
            }
            else if (name == "obsoletes") {
              parseDependencyEntries(& patchPtr->obsoletes, child);
            }
            else if (name == "prerequires") {
              parseDependencyEntries(& patchPtr->prerequires, child);
            }
            else if (name == "requires") {
              parseDependencyEntries(& patchPtr->requires, child);
            }
            else if (name == "recommends") {
              parseDependencyEntries(& patchPtr->recommends, child);
            }
            else if (name == "suggests") {
              parseDependencyEntries(& patchPtr->suggests, child);
            }
            else if (name == "supplements") {
              parseDependencyEntries(& patchPtr->supplements, child);
            }
            else if (name == "enhances") {
              parseDependencyEntries(& patchPtr->enhances, child);
            }
            else if (name == "freshens") {
              parseDependencyEntries(& patchPtr->freshens, child);
            }
            else if (name == "category") {
            patchPtr->category = _helper.content(child);
            }
            else if (name == "reboot_needed") {
            patchPtr->rebootNeeded = true;
            }
            else if (name == "package_manager") {
              patchPtr->packageManager = true;
            }
            else if (name == "update_script") {
              patchPtr->updateScript = _helper.content(child);
            }
            else if (name == "atoms") {
              parseAtomsNode(patchPtr, child);
            }
            else {
              WAR << "XML <data> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
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
            else {
              WAR << "XML <atoms> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      } 
    
      void
      XMLPatchParser::parseAtomNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        shared_ptr<XMLPatchAtomData> atom(new XMLPatchAtomData);
        // inject dependencies and other stuff
        parseResolvableNode( dataPtr, formatNode);        
        dataPtr->atoms.push_back(atom);
      }
      
      void
      XMLPatchParser::parseScriptNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        shared_ptr<XMLPatchScriptData> script(new XMLPatchScriptData);
        parseResolvableNode( dataPtr, formatNode);
        
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
            else
            {
              WAR << "XML <atoms/script> contains the unknown element <" << name << "> " << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        dataPtr->atoms.push_back(script);
      }
      
      void
      XMLPatchParser::parseMessageNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        shared_ptr<XMLPatchMessageData> message(new XMLPatchMessageData);
        
        for (xmlNodePtr child = formatNode->children;  child != 0; child = child ->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "text") {
              message->text.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else {
              WAR << "XML <atoms/message> contains the unknown element <" << name << "> " << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }        
        parseResolvableNode( dataPtr, formatNode);
        dataPtr->atoms.push_back(message);
      }

      void XMLPatchParser::parseResolvableNode(XMLResObjectData_Ptr dataPtr, xmlNodePtr formatNode)
      {
        for (xmlNodePtr child = formatNode->children; child != 0; child = child ->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "name") {
              dataPtr->name = _helper.content(child);
            }
            else if (name == "version") {
              dataPtr->epoch = _helper.attribute(child,"epoch");
              dataPtr->ver = _helper.attribute(child,"ver");
              dataPtr->rel = _helper.attribute(child,"rel");
            }
            else if (name == "arch") {
              dataPtr->arch = _helper.content(child);
            }
            else if (name == "provides") {
              parseDependencyEntries(& dataPtr->provides, child);
            }
            else if (name == "conflicts") {
              parseDependencyEntries(& dataPtr->conflicts, child);
            }
            else if (name == "obsoletes") {
              parseDependencyEntries(& dataPtr->obsoletes, child);
            }
            else if (name == "requires") {
              parseDependencyEntries(& dataPtr->requires, child);
            }
            else if (name == "recommends") {
              parseDependencyEntries(& dataPtr->recommends, child);
            }
            else if (name == "suggests") {
              parseDependencyEntries(& dataPtr->suggests, child);
            }
            else if (name == "supplements") {
              parseDependencyEntries(& dataPtr->supplements, child);
            }
            else if (name == "enhances") {
              parseDependencyEntries(& dataPtr->enhances, child);
            }
            else if (name == "freshens") {
              parseDependencyEntries(& dataPtr->freshens, child);
            }
            else {
              WAR << "XML <atoms/" << _helper.name(formatNode) << "> contains the unknown element <"
                  << name << "> "
                  << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
      }

      
    } // namespace yum
  } // namespace parser
} // namespace zypp
