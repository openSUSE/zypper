/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLResObjectParser.h
 *
*/



#ifndef XMLResObjectParser_h
#define XMLResObjectParser_h

#include <zypp/parser/xmlstore/XMLParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace xmlstore {

      class XMLResObjectParser
      {
      public:
        XMLResObjectParser();
        virtual ~XMLResObjectParser();
    
      protected:
        void parseResObjectCommonData( XMLResObjectData_Ptr dataPtr, xmlNodePtr node);
        void parseDependencies( XMLResObjectData_Ptr dataPtr, xmlNodePtr depNode);
        void parseDependencyEntries(std::list<XMLDependency> *depList,  xmlNodePtr depNode);    
        LibXMLHelper _helper;
      };
    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif
