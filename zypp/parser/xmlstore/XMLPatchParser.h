/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLPatchParser.h
 *
*/

#ifndef XMLPatchParser_h
#define XMLPatchParser_h

#include <zypp/parser/xmlstore/XMLResObjectParser.h>
#include <zypp/parser/xmlstore/XMLParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp
{
  namespace parser
  {
    namespace xmlstore
    {

      class XMLPatchParser : public XMLNodeIterator<XMLPatchData_Ptr>, public XMLResObjectParser
      {
      public:
        XMLPatchParser(std::istream &is, const std::string &baseUrl);
        XMLPatchParser();
        XMLPatchParser(XMLPatchData_Ptr& entry);
        virtual ~XMLPatchParser();
    
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual XMLPatchData_Ptr process(const xmlTextReaderPtr reader);
        void parseAtomsNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode);
        //void parsePackageNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode);
        void parseMessageNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode);
        void parseScriptNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode);
        void parseAtomNode(XMLPatchData_Ptr dataPtr, xmlNodePtr formatNode);
        // base method to parse capabilities and other stuff
        void parseResolvableNode(XMLResObjectData_Ptr dataPtr, xmlNodePtr formatNode);
      };
    } // namespace xml
  } // namespace parser
} // namespace zypp

#endif
