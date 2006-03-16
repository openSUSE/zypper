/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/XMLResObjectParser.h
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
        // FIXME move needed method to a common class, inherit it
        friend class XMLProductParser;
//        friend class XMLPatternParser;
        //void parseResObjectEntries( XMLResObjectData *data,  xmlNodePtr depNode); 
        void parseDependencyEntries(std::list<XMLDependency> *depList,  xmlNodePtr depNode);    
        LibXMLHelper _helper;
      };
    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif
