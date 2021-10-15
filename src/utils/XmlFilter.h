/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef UTILS_XMLFILTER_H
#define UTILS_XMLFILTER_H

#include <zypp/parser/xml/libxmlfwd.h>
#include <zypp/parser/xml/Reader.h>
#include <zypp/base/Xml.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace xml_filter
  {
    ///////////////////////////////////////////////////////////////////
    template <class TKey, class TVal>
    struct TreeNode
    {
      typedef TKey	key_type;
      typedef TVal	value_type;
      typedef size_t	size_type;

      TreeNode()
      {}

      TreeNode( key_type key_r )
      : _key( std::move(key_r) )
      {}

      TreeNode( key_type key_r, value_type value_r )
      : _key( std::move(key_r) )
      , _value( std::move(value_r) )
      {}

      bool empty() const
      { return _children.empty(); }

      size_type size() const
      { return _children.size(); }


      bool isRoot() const
      { return !_parent; }

      bool hasParent() const
      { return _parent; }

      TreeNode & parent()					// std::out_of_range
      { if ( !_parent ) throw std::out_of_range("TreeNode::parent"); return *_parent; }

      const TreeNode & parent() const				// std::out_of_range
      { if ( !_parent ) throw std::out_of_range("TreeNode::parent"); return *_parent; }


      bool hasChild( const key_type & key_r ) const
      { return _children.count( key_r ); }

      TreeNode & at( const key_type & key_r )			// std::out_of_range
      { return _children.at( key_r ); }

      const TreeNode & at( const key_type & key_r ) const	// std::out_of_range
      { return _children.at( key_r ); }

      TreeNode & operator[]( const key_type & key_r )
      { return assertParent( _children[key_r], key_r ); }

      TreeNode & operator[]( key_type && key_r )
      { return assertParent( _children[key_r], std::move(key_r) ); }

      size_type erase( const key_type & key_r )
      { return _children.erase( key_r ); }


      const key_type & key() const
      { return _key; }

      value_type & value()
      { return _value; }

      const value_type & value() const
      { return _value; }

    private:
      template <class Tp>
      TreeNode & assertParent( TreeNode & node_r, Tp && key_r )
      {
        if ( !node_r._parent )
        {
          node_r._parent = this;
          node_r._key = std::forward<Tp>(key_r);
        }
        return node_r;
      }

    private:
      TreeNode *			_parent = nullptr;
      std::map<key_type,TreeNode>	_children;
      key_type				_key;
      value_type			_value;
    };

    template <class TKey, class TVal>
    std::ostream & operator<<( std::ostream & str, const TreeNode<TKey,TVal> & obj )
    { return str << (obj.hasParent()?"N[":"R[") << obj.key() << "|" << obj.size() << "]{" << obj.value() << "}"; }

    template <class TKey, class TVal>
    bool operator==( const TreeNode<TKey,TVal> & lhs, const TreeNode<TKey,TVal> & rhs )
    { return( &lhs == &rhs ); }

    template <class TKey, class TVal>
    bool operator!=( const TreeNode<TKey,TVal> & lhs, const TreeNode<TKey,TVal> & rhs )
    { return !( lhs == rhs ); }

  } // namespace xml_filter
  ///////////////////////////////////////////////////////////////////

  struct XmlFilter
  {
    typedef std::string	Xpath;

    /** Forward partial content defined by Xpath expressions. */
    template <class TContainer>
    static void fwd( std::istream & inp, std::ostream & out, TContainer && xpaths_r )
    { XmlFilter( inp, out, std::forward<TContainer>(xpaths_r) ); }

    static void fwd( std::istream & inp, std::ostream & out, const std::initializer_list<const char *> & xpaths_r )
    { XmlFilter( inp, out, xpaths_r ); }

  private:
    template <class TContainer>
    XmlFilter( std::istream & inp, std::ostream & out, TContainer && xpaths_r )
    : _reader( inp )
    , _out( out )
    {
      for ( auto && xpath : xpaths_r )
      {
        std::vector<std::string> nodes;
        str::split( xpath, back_inserter(nodes), "/" );
        if ( nodes.empty() )
          continue;
        StackNode * t = _top;
        for ( auto && n : nodes )
          t = &((*t)[std::move(n)]);
        t->value() = StackData( true );	// a node to collect
      }

      if ( top().empty() )
        return;		// nothing to collect

      for ( const xml::Node & cnode( *_reader ); ! _reader.atEnd(); _reader.nextNode() )
      {
        switch ( cnode.nodeType() )
        {
          case XML_READER_TYPE_ELEMENT:
          {
            int depth = cnode.depth();
            bool empty = cnode.isEmptyElement();
            std::string cname( cnode.name().asString() );	// will be moved to pushed nodes

            if ( !topData().collecting() )
            {
              if ( !top().hasChild( cname ) )
              {
                // neither collecting nor for a persistent(wanted) node -> discard
                if ( ! empty )
                  _reader.seekToEndNode( depth, cname );
                continue;
              }
              else if ( !top()[cname].value().persistent() )
                topPush( depth, std::move(cname) );		// push to remember
              else
                topPushAndOpen( depth, std::move(cname) );	// push and open
            }
            else
            {
              topPushAndOpen( depth, std::move(cname) );	// push and open
            }

            // collect/remember new top nodes attributes
            while( _reader.nextNodeAttribute() )
              topData().addAttr( { cnode.name().c_str(), cnode.value().c_str() } );

            if ( !empty )
              continue;	// go for END_ELEMENT...
            // else:
            //    isEmptyElement
            //    -> start ELEMENT is also END_ELEMENT
            //    -> fall through to XML_READER_TYPE_END_ELEMENT
          }

          case XML_READER_TYPE_END_ELEMENT:
            topPop();
            break;

          case XML_READER_TYPE_TEXT:
          case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
            if ( topData().collecting() )
            {
              topData().stream() << cnode.value();
            }
            break;

          case XML_READER_TYPE_CDATA:
            if ( topData().collecting() )
            {
              topData().stream() << "<![CDATA[" << cnode.value() << "]]>";
            }
            break;

          case XML_READER_TYPE_NONE:
          default:
            // ignore
            break;
        }
      }
    }

  private:
    struct StackData
    {
      StackData( bool persistent_r = false )
      : _persistent( persistent_r )
      , _collecting( false )
      {}

      bool persistent() const			{ return _persistent; }
      bool collecting() const			{ return _persistent||_collecting; }
      bool open() const				{ return bool(_stream)||bool(_node); }

      std::ostream & open( std::ostream & str_r, std::string name_r, bool collecting_r = true )
      {
        if ( open() ) throw std::logic_error("StackData::open");
        if ( name_r.empty() )
        {
          _stream = &str_r;
        }
        else
        {
          _node.reset( new xmlout::Node( str_r, std::move(name_r), xmlout::Node::optionalContent ) );
          if ( !_attr.empty() )
          {
            for ( auto && attr : _attr )
              _node->addAttr( std::move(attr) );
            _attr.clear();
          }
          if ( _collecting != collecting_r )
            _collecting = collecting_r;
        }
        return str_r;
      }

      std::ostream & stream()
      {
        if ( !_stream )
        {
          if ( !_node ) throw std::logic_error("!StackData::open");
          _stream = &*(*_node);
        }
        return *_stream;
      }

      void close()				{ _node.reset();_stream = nullptr; if ( _collecting ) _collecting = false; }

      void addAttr( xmlout::Node::Attr attr_r )
      {
        if ( _node )
          _node->addAttr( std::move(attr_r) );
        else
          _attr.push_back( std::move(attr_r) );
      }

    private:
      bool	_persistent	:1;		//!< a wanted node (always collecting its subnodes)
      bool	_collecting	:1;		//!< node collecting its subnodes
      std::ostream * _stream = nullptr;
      std::unique_ptr<xmlout::Node> _node;	//!< open node
      std::vector<xmlout::Node::Attr> _attr;	//!< cache attributes if node is not (yet) open
    };

    friend std::ostream & operator<<( std::ostream & str, const StackData & obj )
    { return str << "StackData["
                 << (obj.persistent()?"p":"_")
                 << (obj.collecting()?"c":"_")
                 << (obj.open()?"o":"_")
                 <<"]"; }

    typedef xml_filter::TreeNode<std::string,StackData> StackNode;

    StackNode & top()
    { return *_top; }

    StackData & topData()
    { return top().value(); }

    std::ostream & getParentStream( StackNode & node_r )
    {
      StackData & data( node_r.value() );
      if ( data.open() )
        return data.stream();
      if ( node_r.hasParent() )
      {
        // open parents in non collecting mode
        // TODO: make 'strip root node' optional
        data.open( getParentStream( node_r.parent() ),
                   ( node_r.parent() != _stack ? node_r.key() : std::string() ), /*collecting*/false );
        return data.stream();
      }
      return _out;
    }

    void topPushAndOpen( int depth_r, std::string name_r )
    {
      StackNode & nnode( top()[name_r] );
      StackData & ndata( nnode.value() );
      if ( _top == &_stack )
        name_r.clear();	// TODO: make 'strip root node' optional
      // open new node in collecting mode
      ndata.open( getParentStream( top() ), std::move(name_r), /*collecting*/true );
      // move top
      _top = &nnode;
    }

    void topPush( int depth_r, std::string name_r )
    {
      _top = &top()[std::move(name_r)];
    }

    void topPop()
    {
      StackNode & otop( top() );
      if ( otop.hasParent() )
      {
        StackData & odata( topData() );
        _top = &otop.parent();	// move top
        odata.close();
        if ( otop.empty() && !odata.persistent() )
        {
          top().erase( otop.key() );
        }
      }
    }

  private:
    xml::Reader			_reader;
    std::ostream &		_out;
    StackNode			_stack;
    StackNode *			_top = &_stack;
  };
} // namespace zypp
///////////////////////////////////////////////////////////////////


#endif // UTILS_XMLFILTER_H
