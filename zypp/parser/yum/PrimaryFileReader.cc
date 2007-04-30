/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/TranslatedText.h"
#include "zypp/ByteCount.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PrimaryFileReader::PrimaryFileReader(
      const Pathname &primary_file,
      ProcessPackage callback,
      ParserProgress::Ptr progress)
    :
      _callback(callback), _package(NULL),
      _count(0), _total_packages(0),
      _tag(tag_NONE), _expect_rpm_entry(false), _dtype(zypp::Dep::REQUIRES),
      _progress(progress), _old_progress(0)
  {
    Reader reader( primary_file );
    MIL << "Reading " << primary_file << endl;
    reader.foreachNode( bind( &PrimaryFileReader::consumeNode, this, _1 ) );
  }

  bool PrimaryFileReader::consumeNode(Reader & reader_r)
  {
//    DBG << "**node: " << reader_r->name() << " (" << reader_r->nodeType() << ")" << endl;
    if (_tag == tag_format)
      return consumeFormatChildNodes(reader_r);
  
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "metadata")
      {
        zypp::str::strtonum(reader_r->getAttribute("packages").asString(), _total_packages);
        return true;
      }
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
//        DBG << "packager: " << _package->packager << endl; 
        return true;
      }

      if (reader_r->name() == "url")
      {
//        DBG << "url: " <<  reader_r.nodeText().asString() << endl;
        _package->url = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "time")
      {
        _package->build_time = reader_r->getAttribute("build").asString();
        // ignoring reader_r->getAttribute("file").asString(); (rpm file timestamp)
        return true;
      }

      if (reader_r->name() == "size")
      {
        // ???
        // reader_r->getAttribute("archive").asString();

        // installed size
        ByteCount size(str::strtonum<long long>(reader_r->getAttribute("installed").asString()), Unit());
        _package->size = size;

        // rpm package size
        ByteCount size_rpm(str::strtonum<long long>(reader_r->getAttribute("package").asString()), Unit());
        _package->archive_size = size_rpm;

        return true;
      }

      if (reader_r->name() == "location")
      {
        _package->location = reader_r->getAttribute("href").asString();
        return true;
      }

      if (reader_r->name() == "format")
      {
        _tag = tag_format;
        return true;
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

        // report progress
        long int new_progress = (long int) ((_count/(double) _total_packages)*100);
        if (new_progress - _old_progress >= 5)
        {
          _progress->progress(new_progress);
          _old_progress = new_progress;
        }
        _tag = tag_NONE;
        return true;
      }
    }

    return true;
  }


  // --------------( consume <format> tag )------------------------------------

  bool PrimaryFileReader::consumeFormatChildNodes(Reader & reader_r)
  {
//    DBG << "format subtag: " << reader_r->name() << endl;
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "rpm:entry")
      {
        if (!_expect_rpm_entry)
        {
          // TODO make this a ParseException (once created/taken out of tagfile ns?)
          ZYPP_THROW(Exception("rpm:entry found when not expected"));
        }

        Edition edition(
          reader_r->getAttribute("ver").asString(),
          reader_r->getAttribute("rel").asString(),
          reader_r->getAttribute("epoch").asString()
        );
/*
        DBG << "got rpm:entry for " << _dtype << ": "
            << reader_r->getAttribute("name").asString()
            << " " << edition << endl;
*/
        _package->deps[_dtype].push_back(
          zypp::capability::parse(
            ResTraits<Package>::kind,
            reader_r->getAttribute("name").asString(),
            Rel(reader_r->getAttribute("flags").asString()),
            edition
          )
        );
      }

      if (reader_r->name() == "rpm:license")
      {
        _package->license = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:vendor")
      {
        _package->vendor = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:group")
      {
        _package->group = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:buildhost")
      {
        _package->buildhost = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:sourcerpm")
      {
        //package->source = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:header-range")
      {
        //reader_r->getAttribute("start").asString(),
        //reader_r->getAttribute("end").asString(),
        return true;
      }

      if (reader_r->name() == "rpm:provides")
      {
        _dtype = zypp::Dep::PROVIDES;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:conflicts")
      {
        _dtype = zypp::Dep::CONFLICTS;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:obsoletes")
      {
        _dtype = zypp::Dep::OBSOLETES;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:requires")
      {
        _dtype = zypp::Dep::REQUIRES;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:recommends")
      {
        _dtype = zypp::Dep::RECOMMENDS;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:enhances")
      {
        _dtype = zypp::Dep::ENHANCES;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:supplements")
      {
        _dtype = zypp::Dep::SUPPLEMENTS;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:suggests")
      {
        _dtype = zypp::Dep::SUGGESTS;
        _expect_rpm_entry = true;
        return true;
      }
      if (reader_r->name() == "rpm:suggests")
      {
        _dtype = zypp::Dep::SUGGESTS;
        _expect_rpm_entry = true;
        return true;
      }
      
      if (reader_r->name() == "file")
      {
        // TODO figure out how to read files
        // file = reader_r.nodeText().asString();
        // type = reader_r->getAttribute("type").asString();
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      if (reader_r->name() == "rpm:requires"
          || reader_r->name() == "rpm:provides"
          || reader_r->name() == "rpm:conflicts"
          || reader_r->name() == "rpm:obsoletes"
          || reader_r->name() == "rpm:recommends"
          || reader_r->name() == "rpm:enhances"
          || reader_r->name() == "rpm:supplements"
          || reader_r->name() == "rpm:suggests")
      {
        _expect_rpm_entry = false;
        return true;
      }

      if (reader_r->name() == "format")
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
