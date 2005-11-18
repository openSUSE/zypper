/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMGroupParser.h
 *
*/



#ifndef YUMGroupParser_h
#define YUMGroupParser_h

#include <YUMParserData.h>
#include <XMLNodeIterator.h>
#include <LibXMLHelper.h>
#include <list>

namespace zypp {
  namespace parser {
    namespace yum {

      /**
      *
      * @short Parser for YUM group files.
      *
      * Use this class as an iterator that produces, one after one,
      * YUMGroupDataPtr(s) for the XML group elements.
      * Here's an example:
      *
      * for (YUMGroupParser iter(anIstream, baseUrl),
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
      class YUMGroupParser : public XMLNodeIterator<YUMGroupDataPtr>
      {
      public:
        YUMGroupParser(std::istream &is, const std::string &baseUrl);
        YUMGroupParser();
        YUMGroupParser(YUMGroupDataPtr& entry);
        virtual ~YUMGroupParser();
        
      private:
        virtual bool isInterested(const xmlNodePtr nodePtr);
        virtual YUMGroupDataPtr process(const xmlTextReaderPtr reader);
        void parseGrouplist(YUMGroupDataPtr dataPtr,
                            xmlNodePtr node);
        void parsePackageList(YUMGroupDataPtr dataPtr,
                              xmlNodePtr node);
        
        LibXMLHelper _helper;
      };
    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif
