/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatchesParser.h
 *
*/



#ifndef YUMPatchesParser_h
#define YUMPatchesParser_h

#include <zypp/parser/yum/YUMParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace yum {

      /**
      * @short Parser for YUM primary.xml files (containing package metadata)
      * Use this class as an iterator that produces, one after one,
      * YUMPatchesData_Ptr(s) for the XML package elements in the input.
      * Here's an example:
      *
      * for (YUMPatchesParser iter(anIstream, baseUrl),
      *      iter != YUMOtherParser.end(),     // or: iter() != 0, or ! iter.atEnd()
      *      ++iter) {
      *    doSomething(*iter)
      * }
      *
      * The iterator owns the pointer (i.e., caller must not delete it)
      * until the next ++ operator is called. At this time, it will be
      * destroyed (and a new ENTRYTYPE is created.)
      *
      * If the input is fundamentally flawed so that it makes no sense to
      * continue parsing, XMLNodeIterator will log it and consider the input as finished.
      * You can query the exit status with errorStatus().
      */
      class YUMPatchesParser : public XMLNodeIterator<YUMPatchesData_Ptr>
      {
      public:
        YUMPatchesParser(std::istream &is, const std::string &baseUrl);
        YUMPatchesParser();
        YUMPatchesParser(YUMPatchesData_Ptr& entry);
        virtual ~YUMPatchesParser();
    
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual YUMPatchesData_Ptr process(const xmlTextReaderPtr reader);
        LibXMLHelper _helper;
      };
    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif
