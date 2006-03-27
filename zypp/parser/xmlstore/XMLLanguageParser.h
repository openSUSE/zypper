/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xml/XMLLanguageParser.h
 *
*/



#ifndef XMLLanguageParser_h
#define XMLLanguageParser_h

#include <zypp/parser/xmlstore/XMLResObjectParser.h>
#include <zypp/parser/xmlstore/XMLParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace xmlstore {

      class XMLLanguageParser : public XMLNodeIterator<XMLLanguageData_Ptr>, public XMLResObjectParser
      {
      public:
        XMLLanguageParser(std::istream &is, const std::string &baseUrl);
        XMLLanguageParser();
        XMLLanguageParser(XMLLanguageData_Ptr& entry);
        virtual ~XMLLanguageParser();
        
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual XMLLanguageData_Ptr process(const xmlTextReaderPtr reader);
        
        LibXMLHelper _helper;
      };
    } // namespace xmlstore
  } // namespace parser
} // namespace zypp

#endif
