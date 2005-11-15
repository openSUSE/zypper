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

File:       LibXMLHelper.h

Author:     Michael Radziej <mir@suse.de>
Maintainer: Michael Radziej <mir@suse.de>

Purpose:    Helper class to deal with libxml2 with C++

/-*/
#ifndef LibXMLHelper_h
#define LibXMLHelper_h
#include <string>

extern "C" {
  struct _xmlNode;
  typedef _xmlNode xmlNode;
}

 namespace zypp {

  namespace parser {

    /**
     * @short Easy access to xmlNodes for C++
     */
    
    class LibXMLHelper {
    public:
      /**
       * The default constructor will return an object that does not
       * look into the namespace properties of the nodes. Later, another
       * constructor will be added that takes a list of namespaces as parameters
       * (and maybe also character encoding information), and all nodes of different
       * namespaces will be ignored (i.e., attributes will not be used, and for elements
       * in different namespaces isElement() will return false).
       */
      LibXMLHelper();
    
      /**
       * Destructor
       */
      virtual ~LibXMLHelper();
    
      /**
       * Fetch an attribute
       * @param node the xmlNode
       * @param name name of the attribute
       * @param defaultValue the value to return if this attribute does not exist
       * @return the value of the attribute
       */
      std::string attribute(xmlNode * node, 
                            const std::string &name, 
                            const std::string &defaultValue = std::string()) const;
    
      /**
       * @short The TEXT content of the node and all child nodes
       * Read the value of a node, this can be either the text carried directly by this node if
       * it's a TEXT node or the aggregate string of the values carried by this node child's
       * (TEXT and ENTITY_REF). Entity references are substituted.
       * @param nodePtr the xmlNode
       * @return the content
       */
      std::string content(xmlNode * nodePtr) const;
    
      /**
       * The name of the node
       * @param nodePtr the xmlNode
       * @return the name
       */
      std::string name(const xmlNode * nodePtr) const;
    
      /**
       * returns whether this is an element node (and not, e.g., a attribute or namespace node)
       * @param nodePtr the xmlNode
       * @return true if it is an element node
       */
      bool isElement(const xmlNode * nodePtr) const;
    
      /**
       * returns a string that identifies the position of an element nodes
       * e.g. for error messages
       * @param nodePtr the xmlNode
       * @return the position information
       */
      std::string positionInfo(const xmlNode * nodePtr) const;
    };
  }
}

#endif
