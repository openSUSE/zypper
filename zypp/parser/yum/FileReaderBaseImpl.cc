/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/FileReaderBaseImpl.cc
 * Implementation of shared code for yum::*FileReaders.
 */
//#include <iostream>
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/TranslatedText.h"
#include "zypp/ByteCount.h"

#include "zypp/parser/yum/FileReaderBaseImpl.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  FileReaderBase::BaseImpl::BaseImpl()
    : _expect_rpm_entry(false), _dtype(zypp::Dep::REQUIRES)
  {}

  // --------------------------------------------------------------------------

  bool FileReaderBase::BaseImpl::consumePackageNode(xml::Reader & reader_r, data::Package_Ptr & package_ptr)
  {
    //DBG << "**node: " << reader_r->name() << " (" << reader_r->nodeType() << ") tagpath = " << _tagpath << endl;
    if (isBeingProcessed(tag_format) && consumeFormatNode(reader_r, package_ptr))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "name")
      {
        package_ptr->name = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "arch")
      {
        //if (arch == "src" || arch == "nosrc") arch = "noarch";
        package_ptr->arch = Arch(reader_r.nodeText().asString());
        return true;
      }

      if (reader_r->name() == "version")
      {
        package_ptr->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      if (reader_r->name() == "checksum")
      {
        package_ptr->repositoryLocation.fileChecksum = CheckSum(
                            reader_r->getAttribute("type").asString(),
                            reader_r.nodeText().asString());
        // ignoring pkgid attribute
        return true;
      }

      if (reader_r->name() == "summary")
      {
        package_ptr->summary.setText(
            reader_r.nodeText().asString(),
            Locale(reader_r->getAttribute("lang").asString()));
        return true;
      }

      if (reader_r->name() == "description")
      {
        package_ptr->description.setText(
            reader_r.nodeText().asString(),
            Locale(reader_r->getAttribute("lang").asString()));
        return true;
      }

      if (reader_r->name() == "packager")
      {
        package_ptr->packager = reader_r.nodeText().asString();
//        DBG << "packager: " << package_ptr->packager << endl;
        return true;
      }

      if (reader_r->name() == "url")
      {
//        DBG << "url: " <<  reader_r.nodeText().asString() << endl;
        package_ptr->url = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "time")
      {
        package_ptr->buildTime = reader_r->getAttribute("build").asString();
        // ignoring reader_r->getAttribute("file").asString(); (rpm file timestamp)
        return true;
      }

      if (reader_r->name() == "size")
      {
        // ???
        // reader_r->getAttribute("archive").asString();

        // installed size
        package_ptr->installedSize = str::strtonum<ByteCount::SizeType>( reader_r->getAttribute("installed").asString() );

        // rpm package size
        package_ptr->repositoryLocation.fileSize = str::strtonum<ByteCount::SizeType>( reader_r->getAttribute("package").asString() );

        return true;
      }

      if (reader_r->name() == "location")
      {
        package_ptr->repositoryLocation.filePath = reader_r->getAttribute("href").asString();
        return true;
      }

      if (reader_r->name() == "format")
      {
        tag(tag_format);
        return true;
      }
    }

    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      if (reader_r->name() == "package")
      {
        // indicate that further processing is required
        return false;
      }
    }

    return true;
  }


  // --------------( consume <format> tag )------------------------------------

  bool FileReaderBase::BaseImpl::consumeFormatNode(xml::Reader & reader_r, data::Package_Ptr & package_ptr)
  {
    if (consumeDependency(reader_r, package_ptr->deps))
      // this node has been a dependency, which has been handled by
      // consumeDependency(), so return right away. 
      return true;

//    DBG << "format subtag: " << reader_r->name() << endl;
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "rpm:license")
      {
        package_ptr->license = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:vendor")
      {
        package_ptr->vendor = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:group")
      {
        package_ptr->group = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "rpm:buildhost")
      {
        package_ptr->buildhost = reader_r.nodeText().asString();
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
      if (reader_r->name() == "format")
      {
        toParentTag();
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool FileReaderBase::BaseImpl::consumeDependency(xml::Reader & reader_r, data::Dependencies & deps_r)
  {
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
        
        string kind_str = reader_r->getAttribute("kind").asString();
        Resolvable::Kind kind;
        if (kind_str.empty())
           kind = ResTraits<Package>::kind;
        else
          kind = Resolvable::Kind(kind_str); 
          
/*
        DBG << "got rpm:entry for " << _dtype << ": "
            << reader_r->getAttribute("name").asString()
            << " " << edition << " (" << kind << ")" << endl;
*/
        deps_r[_dtype].insert(
          zypp::capability::parse(
            kind,
            reader_r->getAttribute("name").asString(),
            Rel(reader_r->getAttribute("flags").asString()),
            edition
          )
        );
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
    }

    // tell the caller this has not been a dependency tag (i.e. not processed)
    return false;
  }

  // --------------------------------------------------------------------------

  string FileReaderBase::BaseImpl::TagPath::asString()
  {
    ostringstream s;

    s << "(";

    if (depth())
    {
      TagList::const_iterator p = path.begin();
      s << *p;
      ++p;

      for (; p != path.end(); ++p)
        s << "," << *p;
    }
    else
      s << "empty";

    s << ")";

    return s.str();
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
