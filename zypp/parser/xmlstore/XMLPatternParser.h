/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XMLPatternParser.h
 *
*/



#ifndef XMLPatternParser_h
#define XMLPatternParser_h

#include <zypp/parser/xmlstore/XMLResObjectParser.h>
#include <zypp/parser/xmlstore/XMLParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace xmlstore {

      class XMLPatternParser : public XMLNodeIterator<XMLPatternData_Ptr>, public XMLResObjectParser
      {
      public:
        XMLPatternParser(std::istream &is, const std::string &baseUrl);
        XMLPatternParser();
        XMLPatternParser(XMLPatternData_Ptr& entry);
        virtual ~XMLPatternParser();
        
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual XMLPatternData_Ptr process(const xmlTextReaderPtr reader);
        
        LibXMLHelper _helper;
      };
    } // namespace xmlstore
  } // namespace parser
} // namespace zypp

#endif
