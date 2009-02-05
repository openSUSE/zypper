/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDef.cc
 *
*/
#include <iostream>
#include <sstream>
#include <string>
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/DtorReset.h"
#include "zypp/base/DefaultIntegral.h"

#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/ParseDefException.h"
#include "zypp/parser/xml/ParseDefConsume.h"
#include "zypp/parser/xml/Reader.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefImplConsume
    //
    /** */
    struct ParseDefImplConsume : public ParseDefConsumeRedirect
    {
      virtual void start( const Node & node_r )
      {
        debuglog( "START ", node_r );
        ParseDefConsumeRedirect::start( node_r );
      }

      virtual void text( const Node & node_r )
      {
        debuglog( "TEXT  ", node_r );
        ParseDefConsumeRedirect::text( node_r );
      }

      virtual void cdata( const Node & node_r )
      {
        debuglog( "CDATA ", node_r );
        ParseDefConsumeRedirect::cdata( node_r );
      }

      virtual void done( const Node & node_r )
      {
        debuglog( "DONE  ", node_r );
        ParseDefConsumeRedirect::done( node_r );
      }

      virtual void startSubnode( const Node & node_r )
      {
        debuglog( "--->  ", node_r );
        ParseDefConsumeRedirect::startSubnode( node_r );
      }

      virtual void doneSubnode( const Node & node_r )
      {
        debuglog( "<---  ", node_r );
        ParseDefConsumeRedirect::doneSubnode( node_r );
      }

      void debuglog( const char *const tag_r, const Node & node_r )
      {
        if ( ParseDef::_debug )
          DBG << tag_r << node_r << endl;
      }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDef::Impl
    //
    /** ParseDef implementation.
     * \todo Check using share_ptr_from_this for parent in addNode.
    */
    class ParseDef::Impl
    {
      friend std::ostream & operator<<( std::ostream & str, const ParseDef::Impl & obj );
    public:
      typedef shared_ptr<Impl>               ImplPtr;
      typedef std::map<std::string, ImplPtr> SubNodes;

    public:
      Impl( const std::string & name_r, Mode mode_r, const shared_ptr<ParseDefConsume> & target_r = shared_ptr<ParseDefConsume>() )
      : _name( name_r )
      , _mode( mode_r )
      , _parent( NULL )
      {
        if ( target_r )
          _callback.setRedirect( target_r );
      }

      ~Impl()
      {
        for ( SubNodes::iterator it = _subnodes.begin(); it != _subnodes.end(); ++it )
          {
            it->second->_parent = NULL;
          }
      }

      bool isOptional() const
      { return Traits::ModeBits(_mode).isEqual<Traits::TypeBits>( Traits::BIT_OPTIONAL ); }

      bool isMandatory() const
      { return Traits::ModeBits(_mode).isEqual<Traits::TypeBits>( Traits::BIT_MANDTAORY ); }

      bool singleDef() const
      { return Traits::ModeBits(_mode).isEqual<Traits::VisitBits>( Traits::BIT_ONCE ); }

      bool multiDef() const
      { return Traits::ModeBits(_mode).isEqual<Traits::VisitBits>( Traits::BIT_MULTIPLE ); }

    public:
      void addNode( const ImplPtr & subnode_r );

      ImplPtr getNode( const std::string & name_r ) const
      {
        SubNodes::const_iterator it = _subnodes.find( name_r );
        if ( it != _subnodes.end() )
          return it->second;
        return ImplPtr();
      }

      void take( Reader & reader_r );

    private:
      /** Skip the current node.
       * \pre  Current node must be XML_READER_TYPE_ELEMENT.
       * \post At the corresponding end node.
               (XML_READER_TYPE_END_ELEMENT or atill at the same node,
               if it'a an empty element <tt>&lt;node /&gt;</tt>).
       * \return last call to xml::Reader::nextNode.
       * \throws ParseDefValidateException if no matching end node found.
      */
      bool skipNode( Reader & reader_r );

      std::string exstr( const std::string & what_r, const Impl & impl_r ) const
      {
        std::ostringstream str;
        str << impl_r << ": " << what_r;
        return str.str();
      }
      std::string exstr( const std::string & what_r, const Impl & impl_r, const Reader & reader_r ) const
      {
        std::ostringstream str;
        str << impl_r << ": " << what_r << " |reading " << *reader_r;
        return str.str();
      }

    public:
      std::string                 _name;
      Mode                        _mode;
      DefaultIntegral<unsigned,0> _visited;

      Impl *                      _parent;
      SubNodes                    _subnodes;
      ParseDefImplConsume         _callback;

      DefaultIntegral<int,-1>     _parseDepth;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDef::Impl::addNode
    //	METHOD TYPE : void
    //
    void ParseDef::Impl::addNode( const ImplPtr & subnode_r )
    {
      std::pair<SubNodes::iterator, bool> res
      = _subnodes.insert( std::make_pair( subnode_r->_name, subnode_r ) );

      if ( ! res.second )
        {
          ZYPP_THROW( ParseDefBuildException( exstr("Multiple definiton of subnode "+subnode_r->_name, *this) ) );
        }
      if ( res.first->second->_parent )
        {
          ZYPP_THROW( ParseDefBuildException( exstr("Can not reparent subnode "+subnode_r->_name, *this) ) );
        }
      res.first->second->_parent = this;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDef::Impl::take
    //	METHOD TYPE : void
    //
    void ParseDef::Impl::take( Reader & reader_r )
    {
      if ( reader_r->nodeType() != XML_READER_TYPE_ELEMENT )
        {
          if ( reader_r->depth() == 0 )
          {
            // on the verry first level we skip any initial whitespace and comments...
            do {
              // advance to next node
              if ( ! reader_r.nextNode() )
                {
                  ZYPP_THROW( ParseDefValidateException( exstr( "Unexpected EOF ", *this ) ) );
                }
            } while( reader_r->nodeType() != XML_READER_TYPE_ELEMENT );
          }
          else
          {
            ZYPP_THROW( ParseDefValidateException( exstr("Expected ELEMENT", *this, reader_r) ) );
          }
        }
      if ( reader_r->name() != _name )
        {
          ZYPP_THROW( ParseDefValidateException( exstr("Wrong ELEMENT name", *this, reader_r) ) );
        }
      if ( _visited >= 1 && ! multiDef() )
        {
          ZYPP_THROW( ParseDefValidateException( exstr("Multiple definitions", *this, reader_r) ) );
        }

      ++_visited; // Accepted to parse
      DtorReset x( _parseDepth, -1 );
      _parseDepth = reader_r->depth();

      // Parse attributes
      _callback.start( *reader_r );

      // Get content up to end node
      // Empty element (<node />) has no separate end node, so
      // there's nothing to parse.
      if ( ! reader_r->isEmptyElement() )
        {
          // For non empty elements (<node></node>) parse known nodes
          // text and cdata elelments skip unknown nodes.
          for ( bool done = false; ! done ; /*advance in inside loop*/)
            {
              // advance to next node
              if ( ! reader_r.nextNode() )
                {
                  ZYPP_THROW( ParseDefValidateException( exstr( "Unexpected EOF ", *this ) ) );
                }

              switch ( reader_r->nodeType() )
                {
                case XML_READER_TYPE_ELEMENT:
                  // Parse or skip unknown. Anyway reader is located at the
                  // corresponding end node, or an exception was thrown.
                  {
                    ImplPtr sub( getNode( reader_r->name().asString() ) );
                    if ( sub )
                      {
                        _callback.startSubnode( *reader_r );
                        sub->take( reader_r );
                        _callback.doneSubnode( *reader_r );
                      }
                    else
                      {
                        if ( ParseDef::_debug )
                          WAR << "Skip unknown node " << *reader_r << " in "<< *this << endl;
                        skipNode( reader_r );
                      }
                  }
                break;

                case XML_READER_TYPE_END_ELEMENT:
                  // This must be the corresponding end node!
                  if ( reader_r->depth() == _parseDepth
                       && reader_r->name() == _name )
                    {
                      done = true;
                    }
                  else
                    {
                      ZYPP_THROW( ParseDefValidateException( exstr("unexpected END_ELEMENT name", *this, reader_r) ) );
                    }
                  break;

                case XML_READER_TYPE_TEXT:
                  // collect or skip
                  _callback.text( *reader_r );
                  break;

                case XML_READER_TYPE_CDATA:
                  // collect or skip
                  _callback.cdata( *reader_r );
                  break;

                default:
                  //DBG << exstr("SKIP ", *this, reader_r) << endl;
                  break;
                }
            }
        }

      // Parsing complete. Check whether all mandatory nodes were
      // present. Finally position behind the end node.
      for ( SubNodes::iterator it = _subnodes.begin(); it != _subnodes.end(); ++it )
        {
          if ( ! it->second->_visited && it->second->isMandatory() )
            {
              ZYPP_THROW( ParseDefValidateException( exstr("Mandatory ELEMENT missing", *(it->second), reader_r) ) );
            }
          it->second->_visited = 0; // reset to be ready for an other visit to this!!
        }

      _callback.done( *reader_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDef::Impl::skipNode
    //	METHOD TYPE : void
    //
    bool ParseDef::Impl::skipNode( xml::Reader & reader_r )
    {
      if ( ! reader_r.seekToEndNode( reader_r->depth(),
                                     reader_r->name().asString() ) )
        {
          ZYPP_THROW( ParseDefValidateException
                      ( exstr( str::form( "EOF while looking for [%d] <\\%s>",
                                          reader_r->depth(),
                                          reader_r->name().c_str() ),
                               *this ) ) );
        }
      return true;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ParseDef::Impl & obj )
    {
      return str << "ParseDef(" << obj._name
                 << ", " << obj._mode
                 << ", visits " << obj._visited
                 << ")";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDef
    //
    ///////////////////////////////////////////////////////////////////

    bool ParseDef::_debug = false;

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDef::ParseDef
    //	METHOD TYPE : Ctor
    //
    ParseDef::ParseDef( const std::string & name_r, Mode mode_r )
    : _pimpl( new Impl( name_r, mode_r ) )
    {}

    ParseDef::ParseDef( const std::string & name_r, Mode mode_r, const shared_ptr<ParseDefConsume> & target_r )
    : _pimpl( new Impl( name_r, mode_r, target_r ) )
    {}

    ParseDef::ParseDef( const shared_ptr<Impl> & pimpl_r )
    : _pimpl( pimpl_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ParseDef::~ParseDef
    //	METHOD TYPE : Dtor
    //
    ParseDef::~ParseDef()
    {}

    const std::string & ParseDef::name() const
    { return _pimpl->_name; }

    ParseDef::Mode ParseDef::mode() const
    { return _pimpl->_mode; }

    bool ParseDef::isOptional() const
    { return _pimpl->isOptional(); }

    bool ParseDef::isMandatory() const
    { return _pimpl->isMandatory(); }

    bool ParseDef::singleDef() const
    { return _pimpl->singleDef(); }

    bool ParseDef::multiDef() const
    { return _pimpl->multiDef(); }

    unsigned ParseDef::visited() const
    { return _pimpl->_visited; }

    ParseDef & ParseDef::addNode( ParseDef & subnode_r )
    { _pimpl->addNode( subnode_r._pimpl.getPtr() ); return *this; }

    ParseDef ParseDef::operator[]( const std::string & name_r )
    {
      shared_ptr<Impl> retimpl( _pimpl->getNode( name_r ) );
      if ( ! retimpl )
        {
          ZYPP_THROW( ParseDefBuildException( "No subnode "+name_r ) );
        }
      return retimpl;
    }

    void ParseDef::setConsumer( const shared_ptr<ParseDefConsume> & target_r )
    { _pimpl->_callback.setRedirect( target_r ); }

    void ParseDef::setConsumer( ParseDefConsume * allocatedTarget_r )
    { _pimpl->_callback.setRedirect( allocatedTarget_r ); }

    void ParseDef::setConsumer( ParseDefConsume & target_r )
    { _pimpl->_callback.setRedirect( target_r ); }

    void ParseDef::cancelConsumer()
    { _pimpl->_callback.cancelRedirect(); }

    shared_ptr<ParseDefConsume> ParseDef::getConsumer() const
    { return _pimpl->_callback.getRedirect(); }


    void ParseDef::take( Reader & reader_r )
    { _pimpl->take( reader_r ); }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, ParseDef::Mode obj )
    {
      switch ( obj )
        {
#define X(T) case ParseDef::T: return str << #T
          X(OPTIONAL);
          X(MANDTAORY);
          X(MULTIPLE_OPTIONAL);
          X(MULTIPLE_MANDTAORY);
#undef X
        }
      return str;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ParseDef & obj )
    {
      return str << obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
