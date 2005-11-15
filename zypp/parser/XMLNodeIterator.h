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

  File:       XMLNodeIterator.h

  Author:     Michael Radziej <mir@suse.de>
  Maintainer: Michael Radziej <mir@suse.de>

  Purpose: Provides an iterator interface for XML files

*/

#ifndef XMLNodeIterator_h
#define XMLNodeIterator_h

#include <LibXMLHelper.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <cassert>
#include <iterator>

extern "C" {
  typedef void * xmlTextReaderLocatorPtr;
  struct _xmlNode;
  typedef struct _xmlNode xmlNode;
  typedef xmlNode *xmlNodePtr;

  struct _xmlTextReader;
  typedef _xmlTextReader xmlTextReader;
  typedef xmlTextReader *xmlTextReaderPtr;

  struct _xmlError;
  typedef _xmlError xmlError;
  typedef xmlError *xmlErrorPtr;
}

namespace zypp {

  namespace parser {
   
    
    /**
     * @short class for reporting syntax errors in XMLNodeIterator.
     */
    class XMLParserError {
    public:
      /**
       * Constructor
       */
      XMLParserError(const char *msg,
                     int severity,
                     xmlTextReaderLocatorPtr locator,
                     int docLine,
                     int docColumn) throw();
    
      ~XMLParserError() throw();
    
      /**
       * The message of the errors
       */
      std::string msg() const throw();
    
      /**
       * The severity of this error
       */
      int severity() const throw();
    
      /**
       * See libxml2 documentation
       */
      xmlTextReaderLocatorPtr locator() const throw();
    
      /**
       * The line number in the xml document where the error occurred.
       */
      int docLine() const throw();
    
      /**
       * The column number in the xml document where the error occurred.
       */
      int docColumn() const throw();
    
      /**
       * Gives a string describing the position in the xml document.
       * (either empty, or "at line ..., column ...")
       **/
      std::string position() const throw();
    
    private:
    
      std::string _msg;
      int _severity;
      xmlTextReaderLocatorPtr _locator;
      int _docLine;
      int _docColumn;
    };
    
    
    std::ostream& operator<<(std::ostream &out, const XMLParserError& error);
    
    
    
    /**
     *
     * @short Abstract class to iterate over an xml stream
     *
     * Derive from XMLNodeIterator<ENTRYTYPE> to get an iterator
     * that returns ENTRYTYPE objects. A derived class must provide
     * isInterested() and process(). It should also provide a
     * Constructor Derived(std::stream,std::string baseUrl) which
     * must call fetchNext().
     *
     * The derived iterator class should be compatible with an stl
     * input iterator. Use like this:
     *
     * for (Iterator iter(anIstream, baseUrl),
     *      iter != Iterator.end(),     // or: iter() != 0, or ! iter.atEnd()
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
    
     
    class XMLNodeIteratorBase {
    public:
      /**
       * Constructor. Derived classes must call fetchNext() here.
       * @param input is the input stream (contains the xml stuff)
       * @param baseUrl is the base URL of the xml document
       * FIXME: use XMLParserError::operator<< instead of doing it on my own.
       */
      XMLNodeIteratorBase(std::istream &input, 
                          const std::string &baseUrl,
                          const char *validationPath = 0);
    
      /**
       * Constructor for an empty iterator.
       * An empty iterator is already at its end.
       * This is what end() returns ...
       */
      XMLNodeIteratorBase();
    
      /**
       * Destructor
       */
      virtual ~XMLNodeIteratorBase();
    
      /**
       * Have we reached the end?
       * A parser error also means "end reached"
       * @return whether the end has been reached.
       */
      bool atEnd() const;
    
      /**
       * Two iterators are equal if both are at the end
       * or if they are identical.
       * Since you cannot copy an XMLNodeIterator, everything
       * else is not equal.
       * @param other the other iterator
       * @return true if equal
       */
      bool 
      operator==(const XMLNodeIteratorBase &other) const;
    
      /**
       * Opposit of operator==
       * @param other the other iterator
       * @return true if not equal
       */
      bool 
      operator!=(const XMLNodeIteratorBase &otherNode) const
      {
        return ! operator==(otherNode);
      }
    
      /**
       * returns the error status or 0 if no error
       * the returned pointer is not-owning,
       * it will be deleted upon destruction of the XMLNodeIterator.
       * @return pointer to error status (if exists)
       */
      const XMLParserError *
      errorStatus() const;
    
    protected:
    
      /**
       * filter for the xml nodes
       * The derived class decides which xml nodes it is actually interested in.
       * For each that is selected, process() will be called an the resulting ENTRYTYPE
       * object used as the next value for the iterator.
       * Documentation for the node structure can be found in the libxml2 documentation.
       * Have a look at LibXMLHelper to access node attributes and contents.
       * @param nodePtr points to the xml node in question. Only the node is available, not the subtree.
       *                See libxml2 documentation.
       * @return true if interested
       */
      virtual bool
      isInterested(const xmlNodePtr nodePtr) = 0;
      
      /**
       * process an xml node and set it as next element
       * The derived class has to produce the ENTRYTYPE object here.
       * Details about the xml reader is in the libxml2 documentation.
       * You'll most probably want to use xmlTextReaderExpand(reader) to
       * request the full subtree, and then use the links in the resulting 
       * node structure to traverse, and class LibXMLHelper to access the 
       * attributes and element contents.
       * fetchNext() cannot throw an error since it will be called in the constructor.
       * Instead, in case of a fundamental syntax error the error is saved
       * and will be thrown with the next checkError().
       * @param readerPtr points to the xmlTextReader that reads the xml stream.
       */
      virtual void
      _process(const xmlTextReaderPtr readerPtr) = 0;
      
      /**
       * Fetch the next element and save it as next element
       */
      void fetchNext();
    
      /**
       * Internal function to set the _error variable
       * in case of a parser error. It logs the message
       * and saves errors in _error, so that they will
       * be thrown by checkError().
       * @param arg set to this with xmlReaderSetErrorHandler()
       * @param msg the error message
       * @param severity the severity
       * @param locator as defined by libxml2
       */
      static void
      errorHandler(void * arg,
                   const char * msg,
                   int severity,
                   xmlTextReaderLocatorPtr locator);
    
      
      virtual void setCurrent(const void *data) = 0;
      virtual void* getCurrent() const = 0;
    
    private:
    
      /**
       * assignment is forbidden.
       * Reason: We can't copy an xmlTextReader
       */
      XMLNodeIteratorBase & operator=(const XMLNodeIteratorBase& otherNode);
    
      /**
       * copy constructor is forbidden.
       * Reason: We can't copy an xmlTextReader
       * FIXME: This prevents implementing the end() method for derived classes.
       * 
       * @param otherNode 
       * @return 
       */
      XMLNodeIteratorBase(const XMLNodeIteratorBase& otherNode);
    
      /**
       * if an error occured, this contains the error.
       */
      std::auto_ptr<XMLParserError> _error;
    
      /**
       * contains the istream to read the xml file from.
       * Can be 0 if at end or if the current element is the only element left.
       **/
      std::istream* _input;
      
      /**
       * contains the xmlTextReader used to parse the xml file.
       **/
      xmlTextReaderPtr _reader;
    
      /**
       * contains the base URL of the xml documentation
       */
      std::string _baseUrl;
    }; /* end class XMLNodeIteratorBase */
    
    
    
    /* --------------------------------------------------------------------------- */
    
    template <class ENTRYTYPE>
    class XMLNodeIterator : public XMLNodeIteratorBase,
                            public std::iterator<std::input_iterator_tag, ENTRYTYPE>  {
    public:
      /**
       * Constructor. Derived classes must call fetchNext() here.
       * @param input is the input stream (contains the xml stuff)
       * @param baseUrl is the base URL of the xml document
       * FIXME: use XMLParserError::operator<< instead of doing it on my own.
       */
      XMLNodeIterator(std::istream &input,
                      const std::string &baseUrl,
                      const char *validationPath = 0)
        : XMLNodeIteratorBase(input, baseUrl, validationPath), _current(0)
      {
        /* Derived classes must call fetchNext() in their constructors themselves,
           XMLNodeIterator has no access to their virtual functions during
           construction */
      }
      
      
      /**
       * Constructor for a trivial iterator.
       * A trivial iterator contains only one element.
       * This is at least needed internally for the
       * postinc (iter++) operator
       * @param entry is the one and only element of this iterator.
       */
      XMLNodeIterator(ENTRYTYPE &entry)
        : XMLNodeIteratorBase()
      { 
        setCurrent((void *)& entry);
      }
      
      /**
       * Constructor for an empty iterator.
       * An empty iterator is already at its end.
       * This is what end() returns ...
       */
      XMLNodeIterator()
        : XMLNodeIteratorBase(), _current(0)
      { }
      
      /**
       * Destructor
       */
      virtual ~XMLNodeIterator()
      { }
      
      /**
       * Fetch a pointer to the current element
       * @return pointer to the current element.
       */
      ENTRYTYPE &
        operator*() const
      {
        assert (! atEnd());
        return * (ENTRYTYPE *) getCurrent();
      }
      
      /**
       * Fetch the current element
       * @return the current element
       */
      ENTRYTYPE *
        operator()() const
      {
        if (_error)
          return 0;
        else
          return getCurrent();
      }
      
      /**
       * Go to the next element and return it
       * @return the next element
       */
      XMLNodeIterator<ENTRYTYPE> &  /* ++iter */
      operator++() {
        fetchNext();
        return *this;
      }
      
      /**
       * remember the current element, go to next and return remembered one.
       * avoid this, usually you need the preinc operator (++iter)
       * This function may throw ParserError if something is fundamentally wrong
       * with the input.
       * @return the current element
       */
      XMLNodeIterator operator++(int)   /* iter++ */
      {
        assert (!atEnd());
        XMLNodeIterator<ENTRYTYPE> tmp(operator()());
        fetchNext();
        return tmp;
      }
      
      /**
       * similar to operator*, allows direct member access
       * @return pointer to current element
       */
      const ENTRYTYPE *
        operator->()
      {
        assert(! atEnd());
        return getCurrent();
      }
      
    protected:
      
      /**
       * filter for the xml nodes
       * The derived class decides which xml nodes it is actually interested in.
       * For each that is selected, process() will be called an the resulting ENTRYTYPE
       * object used as the next value for the iterator.
       * Documentation for the node structure can be found in the libxml2 documentation.
       * Have a look at LibXMLHelper to access node attributes and contents.
       * @param nodePtr points to the xml node in question. Only the node is available, not the subtree.
       *                See libxml2 documentation.
       * @return true if interested
       */
      virtual bool
        isInterested(const xmlNodePtr nodePtr) = 0;
      
      /**
       * process an xml node
       * The derived class has to produce the ENTRYTYPE object here.
       * Details about the xml reader is in the libxml2 documentation.
       * You'll most probably want to use xmlTextReaderExpand(reader) to
       * request the full subtree, and then use the links in the resulting
       * node structure to traverse, and class LibXMLHelper to access the
       * attributes and element contents.
       * fetchNext() cannot throw an error since it will be called in the constructor.
       * Instead, in case of a fundamental syntax error the error is saved
       * and will be thrown with the next checkError().
       * @param readerPtr points to the xmlTextReader that reads the xml stream.
       * @return
       */
      virtual ENTRYTYPE
      process(const xmlTextReaderPtr readerPtr) = 0;
    
      void
      _process(const xmlTextReaderPtr readerPtr)
      {
        setCurrent(new ENTRYTYPE(process(readerPtr)));
      }
    
    private:
    
      void setCurrent(const void *data)
      {
        if (data)
          _current.reset(new ENTRYTYPE(* (ENTRYTYPE *) data));
        else
          _current.reset(0);
      }
    
      void *getCurrent() const
      {
        return _current.get();
      }
      
      /**
       * contains the current element of the iterator.
       * a pointer is used to be able to handle non-assigneable ENTRYTYPEs.
       * The iterator owns the element until the next ++ operation.
       * It can be 0 when the end has been reached.
       **/
      std::auto_ptr<ENTRYTYPE> _current;
    }; /* end class XMLNodeIterator */
    
  }
}

#endif
