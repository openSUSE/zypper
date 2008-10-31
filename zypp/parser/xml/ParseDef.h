/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDef.h
 *
*/
#ifndef ZYPP_PARSER_XML_PARSEDEF_H
#define ZYPP_PARSER_XML_PARSEDEF_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/parser/xml/ParseDefTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    class Reader;
    class ParseDefConsume;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDef
    //
    /** Define a xml node structure to parse.
     *
     * An xml file like this:
     * \code
     * <?xml version="1.0" encoding="UTF-8"?>
     * <syscontent>
     *   <ident>
     *     <name>mycollection</name>
     *     <version epoch="0" ver="1.0" rel="1"/>
     *     <description>All the cool stuff...</description>
     *     <created>1165270942</created>
     *   </ident>
     *   <onsys>
     *     <entry kind="package" name="pax" epoch="0" ver="3.4" rel="12" arch="x86_64"/>
     *     <entry kind="product" name="SUSE_SLES" epoch="0" ver="10" arch="x86_64"/>
     *     <entry ...
     *   </onsys>
     * </syscontent>
     * \endcode
     *
     * Could be described by:
     * \code
     * using namespace xml;
     * struct SycontentNode : public ParseDef
     * {
     *   SycontentNode( Mode mode_r )
     *   : ParseDef( "syscontent", mode_r )
     *   {
     *     (*this)("ident",       OPTIONAL)
     *            ("onsys",       OPTIONAL)
     *            ;
     *
     *     (*this)["ident"]
     *            ("name",        OPTIONAL)
     *            ("version",     OPTIONAL)
     *            ("description", OPTIONAL)
     *            ("created",     OPTIONAL)
     *            ;
     *
     *     (*this)["onsys"]
     *            ("entry",       MULTIPLE_OPTIONAL)
     *            ;
     *   }
     * };
     * \endcode
     *
     * To parse it using an \ref xml::Reader:
     * \code
     * xml::Reader reader( input_r );
     * SycontentNode rootNode( xml::ParseDef::MANDTAORY );
     * // Define data consumers here.
     * rootNode.take( reader );
     * \endcode
     *
     * Whithout data consumers this will just parse the file
     * but not retrieve any data. You may attach a consumer
     * derived from \ref xml::ParseDefConsume to each node:
     *
     * \code
     * // Parse Edition from ver/rel/eopch attributes.
     * struct ConsumeEdition : public ParseDefConsume
     * {
     *   ConsumeEdition( Edition & value_r )
     *   : _value( & value_r )
     *   {}
     *
     *   virtual void start( const Node & node_r )
     *   {
     *     *_value = Edition( node_r.getAttribute("ver").asString(),
     *                        node_r.getAttribute("rel").asString(),
     *                        node_r.getAttribute("epoch").asString() );
     *   }
     *
     *   Edition *_value;
     * };
     * \endcode
     * \see \ref xml::ParseDefConsume
     *
     * \code
     * xml::Reader reader( input_r );
     * SycontentNode rootNode( xml::ParseDef::MANDTAORY );
     *
     * // Define data consumers here.
     * Edition _edition;
     * rootNode["ident"]["version"].setConsumer
     * ( new ConsumeEdition( _edition ) );
     *
     * rootNode.take( reader );
     * \endcode
     *
     * That's just one way to collect the data. You could as well
     * use a \ref xml::ParseDefConsumeCallback, and redirect the
     * \c start call to some arbitrary function or method.
    */
    class ParseDef
    {
      typedef ParseDefTraits Traits;

    public:
      enum Mode
        {
          OPTIONAL           = Traits::BIT_OPTIONAL  | Traits::BIT_ONCE,
          MANDTAORY          = Traits::BIT_MANDTAORY | Traits::BIT_ONCE,
          MULTIPLE_OPTIONAL  = Traits::BIT_OPTIONAL  | Traits::BIT_MULTIPLE,
          MULTIPLE_MANDTAORY = Traits::BIT_MANDTAORY | Traits::BIT_MULTIPLE
        };

    public:
      ParseDef( const std::string & name_r, Mode mode_r );
      ParseDef( const std::string & name_r, Mode mode_r, const shared_ptr<ParseDefConsume> & target_r );

      virtual ~ParseDef();

    public:
      const std::string & name() const;
      Mode mode() const;
      bool isOptional() const;
      bool isMandatory() const;
      bool singleDef() const;
      bool multiDef() const;
      unsigned visited() const;

    public:
      /** Add subnode definition.
       * \note As ParseDef copies share their implementation you can
       *       not add the same subnode to multiple parents.
       * \return <tt>*this</tt>.
       * \throws ParseDefBuildException if a subnode with the same name
       *         is already defined, or if the subnode is already
       *         subnode of an other ParseDef.
      */
      ParseDef & addNode( ParseDef & subnode_r );

      ParseDef & addNode( const std::string & name_r, Mode mode_r )
      { ParseDef tmp( name_r, mode_r ); return addNode( tmp ); }

      ParseDef & addNode( const std::string & name_r, Mode mode_r, const shared_ptr<ParseDefConsume> & target_r )
      { ParseDef tmp( name_r, mode_r, target_r ); return addNode( tmp ); }

      /** Add subnode definition.
       * \see addNode.
       */
      ParseDef & operator()( ParseDef & subnode_r )
      { return addNode( subnode_r ); }

      ParseDef & operator()( const std::string & name_r, Mode mode_r )
      { return addNode( name_r, mode_r ); }

      ParseDef & operator()( const std::string & name_r, Mode mode_r, const shared_ptr<ParseDefConsume> & target_r )
      { return addNode( name_r, mode_r, target_r ); }

      /** Get subnode by name.
       * \throws ParseDefBuildException if no subnode with \a name_r exists.
       */
      ParseDef operator[]( const std::string & name_r );

    public:
      /** Set data consumer. */
      void setConsumer( const shared_ptr<ParseDefConsume> & target_r );
      /** Set data consumer.
       * \note \a allocatedTarget_r is immediately wraped into a
       *       shared_ptr.
      */
      void setConsumer( ParseDefConsume * allocatedTarget_r );
      /** Set data consumer. */
      void setConsumer( ParseDefConsume & target_r );
      /** Unset data consumer. */
      void cancelConsumer();

      /** Get data consumer. */
      shared_ptr<ParseDefConsume> getConsumer() const;

      /** Parse the node.
       * This parses the node and all defined subnodes. Unknown
       * subnodes are skipped and leave a warning in the logfile.
       * \pre  Current node must be XML_READER_TYPE_ELEMENT matching
       *       this ParseDefs name.
       * \post All data parsed. At the corresponding end node.
       *       (XML_READER_TYPE_END_ELEMENT or atill at the same node,
       *       if it'a an empty element <tt>&lt;node /&gt;</tt>).
       * \throws ParseDefException on error.
      */
      void take( Reader & reader_r );

    private:
      /** Implementation  */
      class Impl;
      /** Pointer to implementation (shared!) */
      RW_pointer<Impl> _pimpl;

      ParseDef( const shared_ptr<Impl> & pimpl_r );
      friend std::ostream & operator<<( std::ostream & str, const ParseDef & obj );
      friend std::ostream & operator<<( std::ostream & str, const ParseDef::Impl & obj );

    public:
      static bool _debug;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ParseDef ParseDef::Mode stream output. */
    std::ostream & operator<<( std::ostream & str, ParseDef::Mode obj );

    /** \relates ParseDef Stream output. */
    std::ostream & operator<<( std::ostream & str, const ParseDef & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEF_H
