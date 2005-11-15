

#ifndef YUMPrimaryParser_h
#define YUMPrimaryParser_h

#include <YUMParserData.h>
#include <XMLNodeIterator.h>
#include <LibXMLHelper.h>
#include <list>

namespace zypp { namespace parser { namespace YUM {

  /**
  * @short Parser for YUM primary.xml files (containing package metadata)
  * Use this class as an iterator that produces, one after one,
  * YUMPrimaryDataPtr(s) for the XML package elements in the input.
  * Here's an example:
  *
  * for (YUMPrimaryParser iter(anIstream, baseUrl),
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
  class YUMPrimaryParser : public XMLNodeIterator<YUMPrimaryDataPtr>
  {
  public:
    YUMPrimaryParser(std::istream &is, const std::string &baseUrl);
    YUMPrimaryParser();
    YUMPrimaryParser(YUMPrimaryDataPtr& entry);
    virtual ~YUMPrimaryParser();

  private:
    // FIXME move needed method to a common class, inherit it
    friend class YUMPatchParser;
    virtual bool isInterested(const xmlNodePtr nodePtr);
    virtual YUMPrimaryDataPtr process(const xmlTextReaderPtr reader);
    void parseFormatNode(YUMPrimaryDataPtr dataPtr,
                         xmlNodePtr formatNode);
    void parseDependencyEntries(std::list<YUMDependency> *depList, 
                                xmlNodePtr depNode);
    void parseAuthorEntries(std::list<std::string> *authors,
                            xmlNodePtr node);
    void parseKeywordEntries(std::list<std::string> *keywords,
                             xmlNodePtr node);
    void parseDirsizeEntries(std::list<YUMDirSize> *sizes,
                             xmlNodePtr node);

    LibXMLHelper _helper;
  };
}}}

#endif
