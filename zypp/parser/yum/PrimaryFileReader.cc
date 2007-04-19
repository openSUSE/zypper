#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/TranslatedText.h"


using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PrimaryFileReader::PrimaryFileReader(const Pathname &primary_file, ProcessPackage callback)
     : _callback(callback), _package(NULL), _count(0), _tag(tag_NONE)
  {
    Reader reader( primary_file );
    MIL << "Reading " << primary_file << endl;
    reader.foreachNode( bind( &PrimaryFileReader::consumeNode, this, _1 ) );
  }
  
  bool PrimaryFileReader::consumeNode(Reader & reader_r)
  {
    if (_tag = tag_format)
      return consumeFormatChildNodes(reader_r);
  
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "package")
      {
        _tag = tag_package;
  //      DBG << "got " << reader_r->getAttribute("type") << " package" << endl;
        if (_package) delete _package;
        _package = new zypp::data::Package();
  
        return true; 
      }
  
      if (reader_r->name() == "name")
      {
        _package->name = reader_r.nodeText().asString();
        return true;
      }
  
      if (reader_r->name() == "arch")
      {
        //if (arch == "src" || arch == "nosrc") arch = "noarch";
        _package->arch = Arch(reader_r.nodeText().asString());
        return true;
      }
  
      if (reader_r->name() == "version")
      {
        _package->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
      }
      
      if (reader_r->name() == "checksum")
      {
        _package->checksum = CheckSum(reader_r->getAttribute("type").asString(),
                                     reader_r.nodeText().asString());
        // ignoring pkgid attribute 
        return true;
      }
  
      if (reader_r->name() == "summary")
      {
        _package->summary.setText(
            reader_r.nodeText().asString(),
            Locale(reader_r->getAttribute("lang").asString()));
        return true;
      }
  
      if (reader_r->name() == "description")
      {
        _package->description.setText(
            reader_r.nodeText().asString(),
            Locale(reader_r->getAttribute("lang").asString()));
        return true;
      }
  
      if (reader_r->name() == "packager")
      {
        _package->packager = reader_r.nodeText().asString(); 
        return true;
      }
  
      // TODO url
      // TODO time
      // TODO size
  
      if (reader_r->name() == "location")
      {
        _package->location = reader_r->getAttribute("href").asString();
      }
  
      if (reader_r->name() == "format")
      {
        _tag = tag_format;
        consumeFormatChildNodes(reader_r);
      }
    }
    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      if (reader_r->name() == "package")
      {
        _callback(*_package);
        if (_package)
        {
          delete _package;
          _package = NULL;
        }
        _count++;
        _tag = tag_NONE;
      }
      if (reader_r->name() == "metadata")
      {
        MIL << _count << " packages read." << endl;
      }
      return true;
    }
  
    return true;
  }
  
  bool PrimaryFileReader::consumeFormatChildNodes(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      
    }
    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      if (reader_r->name() == "format");
      {
        _tag = tag_package;
        return true;
      }
    }
    return true;
  }


    } // ns yum
  } // ns parser
} //ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
