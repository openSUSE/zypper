#include "Tools.h"
#include <boost/call_traits.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>

#include "zypp/parser/xml/Reader.h"

using namespace std;
using namespace zypp;

#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>
#include <zypp/Pathname.h>
#include <zypp/ExplicitMap.h>
#include <zypp/Depository.h>
#include <zypp/Edition.h>
#include <zypp/CheckSum.h>
#include <zypp/Date.h>

///////////////////////////////////////////////////////////////////

template<class _Cl>
  void ti( const _Cl & c )
  {
    SEC << __PRETTY_FUNCTION__ << endl;
  }

///////////////////////////////////////////////////////////////////
namespace zypp
{
  namespace parser
  {
    namespace yum
    {
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
    }
  }
}
///////////////////////////////////////////////////////////////////

bool nopNode( xml::Reader & reader_r )
{
  return true;
}

bool accNode( xml::Reader & reader_r )
{
  int i;
  xml::XmlString s;
#define X(m) reader_r->m()
      i=X(readState);
      i=X(lineNumber);
      i=X(columnNumber);
      i=X(depth);
      i=X(nodeType);
      s=X(name);
      s=X(prefix);
      s=X(localName);
      i=X(hasAttributes);
      i=X(attributeCount);
      i=X(hasValue);
      s=X(value);
#undef X
      return true;
}

bool dumpNode( xml::Reader & reader_r )
{
  switch ( reader_r->nodeType() )
    {
    case XML_READER_TYPE_ATTRIBUTE:
       DBG << *reader_r << endl;
       break;
    case XML_READER_TYPE_ELEMENT:
       MIL << *reader_r << endl;
       break;
    default:
       WAR << *reader_r << endl;
       break;
    }
  return true;
}

bool dumpNode2( xml::Reader & reader_r )
{
  dumpNode( reader_r );
  return reader_r.foreachNodeAttribute( dumpNode );
}

bool dumpEd( xml::Reader & reader_r )
{
  static int num = 5;
  if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT
       && reader_r->name() == "version" )
    {
      MIL << *reader_r << endl;
      DBG << reader_r->getAttribute( "rel" ) << endl;
      ERR << *reader_r << endl;
      DBG << reader_r->getAttribute( "ver" ) << endl;
      ERR << *reader_r << endl;
      DBG << reader_r->getAttribute( "epoch" ) << endl;
      ERR << *reader_r << endl;
      WAR << Edition( reader_r->getAttribute( "ver" ).asString(),
                      reader_r->getAttribute( "rel" ).asString(),
                      reader_r->getAttribute( "epoch" ).asString() ) << endl;
      --num;
    }
  return num;
}


///////////////////////////////////////////////////////////////////

namespace parser
{
  namespace consume
  {
    struct Repomd
    {
      struct Data
      {
        Depository<std::string> _type;
        Depository<CheckSum>    _checksum;
        Depository<Date>        _timestamp;
        Depository<CheckSum>    _openChecksum;
      };

      Depository<Data> _data;
    };
  }



}

namespace data
{
  struct Repomd
  {
    struct Data
    {
      std::string _type;
      CheckSum    _checksum;
      Date        _timestamp;
      CheckSum    _openChecksum;
    };

    std::map<std::string, Data> _data;
  };
}

///////////////////////////////////////////////////////////////////



struct Element;
std::ostream & operator<<( std::ostream & str, const Element & obj );

struct Element : private base::NonCopyable
{
  Element( xml::Reader & reader_r )
  : _reader( reader_r )
  , _name( _reader->name().asString() )
  , _depth( _reader->depth() )
  {
    MIL << *this << endl;
    //return;
    while( nextElement() )
      {
        Element el( _reader );
      }
  }

  ~Element()
  {
    while( nextElement() )
      { ; }
    DBG << *this << endl;
  }

  bool atBegin() const
  {
    return ( _reader->depth() == _depth
             && _reader->nodeType() == XML_READER_TYPE_ELEMENT
             && _reader->name().c_str() == _name );
  }

  bool atEnd() const
  {
    return ( _reader->depth() == _depth
             && ( _reader->nodeType() == XML_READER_TYPE_END_ELEMENT
                  || ( _reader->nodeType() == XML_READER_TYPE_ELEMENT
                       && _reader->isEmptyElement() ) )
             && _reader->name().c_str() == _name );
  }

  bool nextElement()
  {
    while ( ! atEnd() )
      {
        if ( ! _reader.nextNode() )
          return false;
        if ( _reader->nodeType() == XML_READER_TYPE_ELEMENT )
          return true;
        WAR << *_reader << endl;
      }
    return false;
  }


  xml::Reader & _reader;
  std::string   _name;
  int           _depth;
};

std::ostream & operator<<( std::ostream & str, const Element & obj )
{
  str << ( obj.atBegin() ? 'B' : '_' )
      << ( obj.atEnd() ? 'E' : '_' )
      << obj._depth << ":" <<  std::string( obj._depth, ' ') << obj._name
      << " {" << *obj._reader << '}';
  return str;
}

bool dumpEl( xml::Reader & reader_r )
{
  Element el( reader_r );
  return true;
}

void parse2( const InputStream & file_r )
{
  Measure x( file_r.name() );
  try
    {
      MIL << file_r << endl;
      xml::Reader r( file_r );
      MIL << *r << endl;
      Element el( r );
      MIL << *r << endl;
    }
  catch ( const Exception & )
    {
    }
}
///////////////////////////////////////////////////////////////////
class BasicParser
{
  public:
    typedef function<void( xml::Reader & )>   Consumer;
    typedef ExplicitMap<std::string,Consumer> ConsumerMap;

    BasicParser( const InputStream & file_r )
    : _reader( file_r )
    , _cmap( nop )
    {}

  public:
    xml::Reader & reader()
    { return _reader; }

    const xml::Reader & reader() const
    { return _reader; }

    ConsumerMap & cmap()
    { return _cmap; }

    const ConsumerMap & cmap() const
    { return _cmap; }

  public:

    bool parse( xml::Reader & reader_r )
    {
      switch ( reader_r->nodeType() )
        {
        case XML_READER_TYPE_ELEMENT:
        case XML_READER_TYPE_TEXT:
        case XML_READER_TYPE_CDATA:
        case XML_READER_TYPE_END_ELEMENT:
          consume( reader_r );
        default:
          ;
        }
      return true;
    }

  public:
    void consume( xml::Reader & reader_r, const std::string & key_r )
    { _cmap[key_r]( reader_r ); }

    void consume( xml::Reader & reader_r )
    { consume( reader_r, reader_r->name().asString() ); }

    void consume()
    { consume( _reader ); }

  public:
    static void nop( xml::Reader & reader_r )
    { ; }

    static void log( xml::Reader & reader_r )
    { DBG << "NOP " << *reader_r << endl; }


  protected:
    xml::Reader _reader;
    ConsumerMap _cmap;
};

///////////////////////////////////////////////////////////////////

struct RepomdParser : private BasicParser
{
  RepomdParser( const InputStream & file_r )
  : BasicParser( file_r )
  {
    reader().foreachNode( ref(*this) );
  }

  bool operator()( xml::Reader & reader_r )
  {
    return parse( reader_r );
  }

  // READER goes here!
};

///////////////////////////////////////////////////////////////////
struct Consume
{
  struct Entry
  {
    Pathname _location;
    CheckSum _checksum;
    //unused: Date     _timestamp;
    //unused: CheckSum _openChecksum;
  };

  typedef void (Consume::*Consumer)( xml::Reader & reader_r );

  Consume( const InputStream & file_r )
  : _reader( file_r )
  , _consume( &Consume::nop )
  , _centry( NULL )
  {
    _consume.set( "data", &Consume::data );
    _reader.foreachNode( ref(*this) );
  }

  bool operator()( xml::Reader & reader_r )
  {
    switch ( reader_r->nodeType() )
      {
      case XML_READER_TYPE_ELEMENT:
        (this->*_consume[reader_r->name().asString()])( reader_r );
        //data( reader_r );
        break;
      default:
        WAR << *_reader << endl;
        break;
      }
    return true;
  }

  void nop( xml::Reader & reader_r )
  { ; }

  void log( xml::Reader & reader_r )
  { DBG << "NOP " << *_reader << endl; }

  void data( xml::Reader & reader_r )
  {
    MIL << *_reader << endl;
    _result[reader_r->name().asString()] = Entry();
  }



  xml::Reader _reader;
  ExplicitMap<std::string,Consumer> _consume;
  std::map<std::string,Entry> _result;
  Entry * _centry;
};

std::ostream & operator<<( std::ostream & str, const Consume & obj )
{
  return str;
}

std::ostream & operator<<( std::ostream & str, const Consume::Entry & obj )
{
  return str << "Entry";
}

void parse( const InputStream & file_r )
{
  Measure x( file_r.name() );
  try
    {
      MIL << file_r << endl;
      RepomdParser a( file_r );
      //WAR << a._result << endl;
    }
  catch ( const Exception & )
    {
    }
}

struct Test
{
  struct Mix
  {
    Mix()
    : a( 0 )
    {}

    void seta( int v )
    { a = v; }

    void setb( const string & v )
    { b = v; }

    int    a;
    string b;
   };

  Test()
  : a( 0 )
  {}

  int    a;
  string b;
  Mix    c;
};

std::ostream & operator<<( std::ostream & str, const Test & obj )
{
  return str << "Test(" << obj.a << '|' << obj.b
             << '|' << obj.c.a << '|' << obj.c.b << ')';
}

struct Transform
{
  Transform()
  : outfile( "susedu.xml", std::ios_base::out )
  {}

  static const bool indented = !false;
  static const bool shorttags = !true;
  std::fstream outfile;

  bool operator()( xml::Reader & reader_r )
  {
    switch ( reader_r->nodeType() )
      {
      case XML_READER_TYPE_ELEMENT:
        process( reader_r, true );
        break;
      case XML_READER_TYPE_END_ELEMENT:
        process( reader_r, false );
        break;
      default:
        //WAR << *reader_r << endl;
        break;
      }
    return true;
  }

  struct File
  {
    std::string name;
    std::string type;

    bool operator<( const File & rhs ) const
    { return( name < rhs.name ); }
  };

  struct Package
  {
    std::string    pkgid;
    std::string    name;
    std::string    epoch;
    std::string    ver;
    std::string    rel;
    std::string    arch;
    std::set<File> files;
  };

  shared_ptr<Package> pkg;

  void process( xml::Reader & reader_r, bool open_r )
  {
    if ( reader_r->name() == "file" )
      {
        if ( open_r )
          addFile( reader_r );
      }
    else if ( reader_r->name() == "version" )
      {
        if ( open_r )
          addVersion( reader_r );
      }
    else if ( reader_r->name() == "package" )
      {
        if ( open_r )
          startPackage( reader_r );
        else
          endPackage();
      }
    else if ( reader_r->name() == "filelists" )
      {
        DBG << *reader_r << endl;
        if ( open_r )
          {
            DBG << outfile << endl;
            outfile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
            outfile << "<susedu>" << endl;
          }
        else
          {
            outfile << "</susedu>" << endl;
            outfile.close();
          }
      }
    else
      {
        throw;
      }
  }

  void startPackage( xml::Reader & reader_r )
  {
    endPackage();
    pkg.reset( new Package );
    pkg->pkgid = reader_r->getAttribute( "pkgid" ).asString();
    pkg->name = reader_r->getAttribute( "name" ).asString();
    pkg->arch = reader_r->getAttribute( "arch" ).asString();
  }

  void addVersion( xml::Reader & reader_r )
  {
    pkg->epoch = reader_r->getAttribute( "epoch" ).asString();
    pkg->ver = reader_r->getAttribute( "ver" ).asString();
    pkg->rel = reader_r->getAttribute( "rel" ).asString();
  }

  void addFile( xml::Reader & reader_r )
  {
    File f;
    f.type = reader_r->getAttribute( "type" ).asString();
    for( reader_r.nextNode();
         reader_r->nodeType() != XML_READER_TYPE_END_ELEMENT;
         reader_r.nextNode() )
      {
        if ( reader_r->nodeType() == XML_READER_TYPE_TEXT )
          {
            f.name = reader_r->value().asString();
          }
      }
    pkg->files.insert( f );
  }

  void endPackage()
  {
    if ( pkg )
      {
        writePackage( outfile );
        pkg.reset();
      }
  }

  static std::ostream & putAttr( std::ostream & stream_r,
                                 const std::string & tag_r,
                                 const std::string & value_r )
  {
    if ( value_r.empty() || tag_r.empty() )
      return stream_r;
    return stream_r
           << str::form( " %s=\"%s\"", tag_r.c_str(), value_r.c_str() );
  }

  void writePackage( std::ostream & stream_r )
  {
    stream_r << " <package";
    putAttr( stream_r, "pkgid", pkg->pkgid );
    putAttr( stream_r, "name", pkg->name );
    putAttr( stream_r, "arch", pkg->arch );
    stream_r << ">\n";

    stream_r << "  <version";
    putAttr( stream_r, "epoch", pkg->epoch );
    putAttr( stream_r, "ver", pkg->ver );
    putAttr( stream_r, "rel", pkg->rel );
    stream_r << "/>\n";

    writePackageFiles2( stream_r );

    stream_r << " </package>\n";
  }

  void writePackageFiles( std::ostream & stream_r )
  {
    for ( std::set<File>::const_iterator it = pkg->files.begin();
          it != pkg->files.end(); ++it )
      {
        stream_r << "   <file";
        putAttr( stream_r, "type", it->type );
        stream_r << ">" << it->name << "</file>\n";
      }
  }

  struct Fnode
  {
    Fnode( const std::string & name_r )
    : name( name_r )
    , entry( NULL )
    {}

    std::string             name;
    mutable const File *    entry;
    mutable std::set<Fnode> children;

    const Fnode * add( const std::string & sub_r ) const
    {
      std::set<Fnode>::iterator i = children.find( sub_r );
      if ( i != children.end() )
        return &(*i);
      return &(*(children.insert( Fnode( sub_r ) ).first));
    }

    void dump( std::ostream & stream_r, const std::string & pname, unsigned level ) const
    {
      std::string tname;
      if ( pname.empty() )
        {
          tname = name;
        }
      else if ( pname == "/" )
        {
          tname = pname+name;
        }
      else
        {
          tname = pname+"/"+name;
        }

      if ( children.size() == 1 )
        {
          children.begin()->dump( stream_r, tname, (indented?level:0) );
          return;
        }

      std::string tag;
      stream_r << std::string( level, ' ' );

      if ( entry )
        {
          tag = (shorttags ? "f" : "file");
          stream_r << "<" << tag;
          putAttr( stream_r, (shorttags ? "t" : "type"), entry->type );
          putAttr( stream_r, (shorttags ? "n" : "name"), tname );
        }
      else
        {
          tag = (shorttags ? "b" : "base");
          stream_r << "<" << tag;
          putAttr( stream_r, (shorttags ? "n" : "name"), tname );
        }

      if ( children.empty() )
        {
          stream_r << "/>" << (indented?"\n":"");
        }
      else
        {
          stream_r << ">" << (indented?"\n":"");
          for ( std::set<Fnode>::const_iterator it = children.begin();
                it != children.end(); ++it )
            {
              it->dump( stream_r, "", (indented?level+1:0) );
            }
          stream_r << std::string( level, ' ' ) << "</" << tag << ">" << (indented?"\n":"");
        }
    }

    bool operator<( const Fnode & rhs ) const
    { return( name < rhs.name ); }
  };

  void writePackageFiles2( std::ostream & stream_r )
  {
    Fnode root( "" );
    for ( std::set<File>::const_iterator it = pkg->files.begin();
          it != pkg->files.end(); ++it )
      {
        std::list<std::string> words;
        str::split( it->name, std::back_inserter(words), "/" );

        const Fnode * c = &root;
        for ( std::list<std::string>::const_iterator w = words.begin();
              w != words.end(); ++w )
          {
            c = c->add( *w );
          }
        c->entry = &(*it);
      }
    root.dump( stream_r, "/", (indented?3:0) );
  }

};

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;
  {
    Measure x;
    Pathname repodata( "/Local/PATCHES/repodata" );
    //repodata = "/Local/FACTORY/repodata";
    xml::Reader reader( repodata/"filelists.xml" );
    Transform t;
    reader.foreachNode( ref(t) );
  }
  INT << "===[END]============================================" << endl << endl;
  return 0;
  int s;

  Pathname repodata( "/Local/PATCHES/repodata" );
  //repodata = "/Local/FACTORY/repodata";
  InputStream x ( "/Local/PATCHES/repodata" );
  parse2( repodata/"repomd.xml" );
  //parse2( repodata/"primary.xml" );

  INT << "===[END]============================================" << endl << endl;
  return 0;
  {
    Measure x;
    for ( int i = 1; i; --i )
      {
        parse( repodata/"repomd.xml" );
        parse( repodata/"primary.xml" );
        parse( repodata/"filelists.xml" );
        parse( repodata/"other.xml" );
      }
  }
  ERR << "done..." << endl;
  cin >> s;
  return 0;
  {
    Measure x;
    for ( int i = 20; i; --i )
      {
        parse( (repodata/"repomd.xml").asString() );
        parse( repodata/"primary.xml" );
        parse( repodata/"filelists.xml" );
        parse( repodata/"other.xml" );
      }
  }
  ERR << "done..." << endl;
  cin >> s;

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

