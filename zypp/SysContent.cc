/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SysContent.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/SysContent.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/xml/ParseDef.h"
#include "zypp/parser/xml/ParseDefConsume.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace syscontent
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace // Writer helpers
    { /////////////////////////////////////////////////////////////////

      /** writeXml helper.
       * \return <tt>tag="value"</tt> if value not empty, else
       * an empty string.
      */
      inline std::string attrIf( const std::string & tag_r,
                                 const std::string & value_r )
      {
        std::string ret;
        if ( ! value_r.empty() )
          {
            ret += " ";
            ret += tag_r;
            ret += "=\"";
            ret += value_r;
            ret += "\"";
          }
        return ret;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Writer::Impl
    //
    /** \see Writer */
    class Writer::Impl
    {
    public:
      std::ostream & writeXml( std::ostream & str ) const;

    public:
      std::string _name;
      Edition     _edition;
      std::string _description;
      StorageT    _onsys;

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Writer::Impl::writeXml
    //	METHOD TYPE : std::ostream &
    //
    std::ostream & Writer::Impl::writeXml( std::ostream & str ) const
    {
      // intro
      str << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      str << "<syscontent>\n";
      // ident data
      str << "  <ident>\n";
      str << "    <name>" << _name << "</name>\n";
      str << "    <version"
          << attrIf( "epoch", str::numstring(_edition.epoch()) )
          << attrIf( "ver",   _edition.version() )
          << attrIf( "rel",   _edition.release() )
          << "/>\n";
      str << "    <description>" << _description << "</description>\n";
      str << "    <created>" << Date::now().asSeconds() << "</created>\n";
      str << "  </ident>\n";
      // ResObjects
      str << "  <onsys>\n";
      for ( iterator it = _onsys.begin(); it != _onsys.end(); ++it )
        {
          str << "    <entry"
              << attrIf( "kind",  (*it)->kind().asString() )
              << attrIf( "name",  (*it)->name() )
              << attrIf( "epoch", str::numstring((*it)->edition().epoch()) )
              << attrIf( "ver",   (*it)->edition().version() )
              << attrIf( "rel",   (*it)->edition().release() )
              << attrIf( "arch",  (*it)->arch().asString() )
              << "/>\n";
        }
      str << "  </onsys>\n";
      // extro
      str << "</syscontent>" << endl;
      return str;
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Writer
    //
    ///////////////////////////////////////////////////////////////////

    Writer::Writer()
    : _pimpl( Impl::nullimpl() )
    {}

    const std::string & Writer::name() const
    { return _pimpl->_name; }

    Writer & Writer::name( const std::string & val_r )
    { _pimpl->_name = val_r; return *this; }

    const Edition & Writer::edition() const
    { return _pimpl->_edition; }

    Writer & Writer::edition( const Edition & val_r )
    { _pimpl->_edition = val_r; return *this; }

    const std::string & Writer::description() const
    { return _pimpl->_description; }

    Writer & Writer::description( const std::string & val_r )
    { _pimpl->_description = val_r; return *this; }

    void Writer::addInstalled( const PoolItem & obj_r )
    {
      if ( obj_r.status().isInstalled() )
        {
          _pimpl->_onsys.insert( obj_r.resolvable() );
        }
    }

    void Writer::addIf( const PoolItem & obj_r )
    {
      if ( obj_r.status().isInstalled() != obj_r.status().transacts()
           && ! ( obj_r.status().transacts() && obj_r.status().isBySolver() ) )
        {
          _pimpl->_onsys.insert( obj_r.resolvable() );
        }
    }

    void Writer::add( const ResObject::constPtr & obj_r )
    { _pimpl->_onsys.insert( obj_r ); }

    bool Writer::empty() const
    { return _pimpl->_onsys.empty(); }

    Writer::size_type Writer::size() const
    { return _pimpl->_onsys.size(); }

    Writer::const_iterator Writer::begin() const
    { return _pimpl->_onsys.begin(); }

    Writer::const_iterator Writer::end() const
    { return _pimpl->_onsys.end(); }

    std::ostream & Writer::writeXml( std::ostream & str ) const
    { return _pimpl->writeXml( str ); }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader::Entry::Impl
    //
    class Reader::Entry::Impl
    {
    public:
      std::string _kind;
      std::string _name;
      Edition     _edition;
      Arch        _arch;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader::Entry
    //
    ///////////////////////////////////////////////////////////////////

    Reader::Entry::Entry()
    : _pimpl( new Impl )
    {}

    Reader::Entry::Entry( const shared_ptr<Impl> & pimpl_r )
    : _pimpl( pimpl_r )
    {}

    const std::string & Reader::Entry::kind() const
    { return _pimpl->_kind; }

    const std::string & Reader::Entry::name() const
    { return _pimpl->_name; }

    const Edition & Reader::Entry::edition() const
    { return _pimpl->_edition; }

    const Arch & Reader::Entry::arch() const
    { return _pimpl->_arch; }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader::Impl
    //
    /** \see Reader */
    class Reader::Impl
    {
    public:
      Impl()
      {}

      Impl( std::istream & input_r );

    public:
      std::string _name;
      Edition     _edition;
      std::string _description;
      Date        _created;

      std::list<Entry> _content;

    public:
      /** Offer default Impl. */
      static shared_ptr<Impl> nullimpl()
      {
        static shared_ptr<Impl> _nullimpl( new Impl );
        return _nullimpl;
      }

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace // Reader helpers
    { /////////////////////////////////////////////////////////////////

      using namespace xml;

      /** Sycontent xml node structure. */
      struct SycontentNode : public ParseDef
      {
        SycontentNode( Mode mode_r )
        : ParseDef( "syscontent", mode_r )
        {
          (*this)("ident",       OPTIONAL)
                 ("onsys",       OPTIONAL)
                 ;

          (*this)["ident"]
                 ("name",        OPTIONAL)
                 ("version",     OPTIONAL)
                 ("description", OPTIONAL)
                 ("created",     OPTIONAL)
                 ;

          (*this)["onsys"]
                 ("entry",       MULTIPLE_OPTIONAL)
                 ;
        }
      };

      /** Parse Edition from ver/rel/eopch attributes. */
      struct ConsumeEdition : public ParseDefConsume
      {
        ConsumeEdition( Edition & value_r )
        : _value( & value_r )
        {}

        virtual void start( const Node & node_r )
        {
          *_value = Edition( node_r.getAttribute("ver").asString(),
                             node_r.getAttribute("rel").asString(),
                             node_r.getAttribute("epoch").asString() );
        }

        Edition *_value;
      };

      /** Parse std::string from node value. */
      struct ConsumeString : public ParseDefConsume
      {
        ConsumeString( std::string & value_r )
        : _value( & value_r )
        {}

        virtual void text( const Node & node_r )
        {
          *_value = node_r.value().asString();
        }

        std::string *_value;
      };

      /** Parse Date from node value. */
      struct ConsumeDate : public ParseDefConsume
      {
        ConsumeDate( Date & value_r )
        : _value( & value_r )
        {}

        virtual void text( const Node & node_r )
        {
          *_value = Date(node_r.value().asString());
        }

        Date *_value;
      };

      /** Parse entry list. */
      struct ConsumeEntries : public ParseDefConsume
      {
        ConsumeEntries( std::list<Reader::Entry> & value_r )
        : _value( & value_r )
        {}

        virtual void start( const Node & node_r )
        {
          shared_ptr<Reader::Entry::Impl> centry( new Reader::Entry::Impl );

          centry->_kind = node_r.getAttribute("kind").asString();
          centry->_name = node_r.getAttribute("name").asString();
          centry->_edition = Edition( node_r.getAttribute("ver").asString(),
                                      node_r.getAttribute("rel").asString(),
                                      node_r.getAttribute("epoch").asString() );
          centry->_arch = Arch( node_r.getAttribute("arch").asString() );

          _value->push_back( Reader::Entry( centry ) );
        }

        std::list<Reader::Entry> *_value;
      };

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Reader::Impl::Impl
    //	METHOD TYPE : Constructor
    //
    Reader::Impl::Impl( std::istream & input_r )
    {
      xml::Reader reader( input_r );
      SycontentNode rootNode( xml::ParseDef::MANDTAORY );

      rootNode["ident"]["name"].setConsumer
      ( new ConsumeString( _name ) );

      rootNode["ident"]["version"].setConsumer
      ( new ConsumeEdition( _edition ) );

      rootNode["ident"]["description"].setConsumer
      ( new ConsumeString( _description ) );

      rootNode["ident"]["created"].setConsumer
      ( new ConsumeDate( _created ) );

      rootNode["onsys"]["entry"].setConsumer
      ( new ConsumeEntries( _content ) );

      // parse
      rootNode.take( reader );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Reader
    //
    ///////////////////////////////////////////////////////////////////

    Reader::Reader()
    : _pimpl( Impl::nullimpl() )
    {}

    Reader::Reader( std::istream & input_r )
    : _pimpl( new Impl( input_r ) )
    {}

    const std::string & Reader::name() const
    { return _pimpl->_name; }

    const Edition & Reader::edition() const
    { return _pimpl->_edition; }

    const std::string & Reader::description() const
    { return _pimpl->_description; }

    const Date & Reader::ctime() const
    { return _pimpl->_created; }

    bool Reader::empty() const
    { return _pimpl->_content.empty(); }

    Reader::size_type Reader::size() const
    { return _pimpl->_content.size(); }

    Reader::const_iterator Reader::begin() const
    { return _pimpl->_content.begin(); }

    Reader::const_iterator Reader::end() const
    { return _pimpl->_content.end(); }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : inline std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Reader & obj )
    {
      return str << "syscontent(" << obj.name() << "-" << obj.edition()
                 << ", " << obj.size() << " entries"
                 << ",  created " << obj.ctime()
                 << ")";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace syscontent
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
