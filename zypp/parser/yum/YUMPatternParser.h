/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatternParser.h
 *
*/



#ifndef YUMPatternParser_h
#define YUMPatternParser_h

#include <zypp/parser/yum/YUMParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace yum {

      /**
      *
      * @short Parser for YUM pattern files.
      *
      * Use this class as an iterator that produces, one after one,
      * YUMPatternData_Ptr(s) for the XML pattern elements.
      * Here's an example:
      *
      * for (YUMPatternParser iter(anIstream, baseUrl),
      *      iter != YUMFileListParser.end(),     // or: iter() != 0, or ! iter.atEnd()
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
      class YUMPatternParser : public XMLNodeIterator<YUMPatternData_Ptr>
      {
      public:
        YUMPatternParser(std::istream &is, const std::string &baseUrl);
        YUMPatternParser();
        YUMPatternParser(YUMPatternData_Ptr& entry);
        virtual ~YUMPatternParser();
        
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual YUMPatternData_Ptr process(const xmlTextReaderPtr reader);
        void parsePatternlist(YUMPatternData_Ptr dataPtr,
                            xmlNodePtr node);
        void parsePackageList(YUMPatternData_Ptr dataPtr,
                              xmlNodePtr node);
        
        LibXMLHelper _helper;
      };
    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif
