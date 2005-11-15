/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

File:       YUMOtherParser.h

Author:     Michael Radziej <mir@suse.de>
Maintainer: Michael Radziej <mir@suse.de>

Purpose:    Parses other.xml files in a YUM repository

/-*/

#ifndef YUMOtherParser_h
#define YUMOtherParser_h


#include <YUMParserData.h>
#include <XMLNodeIterator.h>
#include <LibXMLHelper.h>
#include <list>

namespace zypp { namespace parser { namespace YUM {

  /**
  * @short Parser for YUM other.xml files
  * Use this class as an iterator that produces, one after one,
  * YUMOtherDataPtr(s) for the XML package elements.
  * Here's an example:
  * 
  * for (YUMOtherParser iter(anIstream, baseUrl),
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
  class YUMOtherParser : public XMLNodeIterator<YUMOtherDataPtr>
  {
  public:
    /**
     * Constructor.
     * @param is the istream to read from
     * @param baseUrl the base URL of the XML document. Can be left empty.
     */
    YUMOtherParser(std::istream &is, const std::string &baseUrl);

    YUMOtherParser();
    YUMOtherParser(YUMOtherDataPtr& entry);
    
    /**
     * Destructor.
     */
    virtual ~YUMOtherParser();
    
  private:
    /**
     * decides if the parser is interested in the node (and subtree) of an element.
     * @param nodePtr the XML node
     * @return true if the parser is interested.
     */
    virtual bool isInterested(const xmlNodePtr nodePtr);
    
    /**
     * creates a new object from the xml subtree
     * @param reader 
     * @return 
     */
    virtual YUMOtherDataPtr process(const xmlTextReaderPtr reader);

    /**
     * converts the xml stuff to c++ stuff and filters the right namespaces
     */
    LibXMLHelper _helper;
  };
}}}

#endif
