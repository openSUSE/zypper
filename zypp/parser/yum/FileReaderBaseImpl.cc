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
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"
#include "zypp/Arch.h"
#include "zypp/Edition.h"
#include "zypp/TranslatedText.h"
#include "zypp/ByteCount.h"

#include "zypp/parser/ParseException.h"
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

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   *
   * // xpath: <xpath> (?|*|+)
   *
   * if multiplicity is ommited, then the node has multiplicity 'one'.
   */

  // --------------------------------------------------------------------------

  bool FileReaderBase::BaseImpl::consumePackageNode(xml::Reader & reader_r, data::Package_Ptr & package_ptr)
  {
    // DBG << "**node: " << reader_r->name() << " (" << reader_r->nodeType() << ") tagpath = " << _tagpath << endl;
    if (!isBeingProcessed(tag_package))
      ZYPP_THROW(ParseException("consumePackageNode() called outside of package element"));

    if (isBeingProcessed(tag_format) && consumeFormatNode(reader_r, package_ptr))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: //package/name
      if (reader_r->name() == "name")
      {
        package_ptr->name = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //package/arch
      if (reader_r->name() == "arch")
      {
        //if (arch == "src" || arch == "nosrc") arch = "noarch";
        package_ptr->arch = Arch(reader_r.nodeText().asString());
        return true;
      }

      // xpath: //package/version
      if (reader_r->name() == "version")
      {
        package_ptr->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      // xpath: //package/checksum
      if (reader_r->name() == "checksum")
      {
        package_ptr->repositoryLocation.fileChecksum = CheckSum(
                            reader_r->getAttribute("type").asString(),
                            reader_r.nodeText().asString());
        // ignoring pkgid attribute
        return true;
      }

      // xpath: //package/summary (*)
      if (reader_r->name() == "summary")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        package_ptr->summary.setText(
            reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: //package/description (*)
      if (reader_r->name() == "description")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        package_ptr->description.setText(
            reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: //package/packager 
      if (reader_r->name() == "packager")
      {
        package_ptr->packager = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //package/url
      if (reader_r->name() == "url")
      {
        package_ptr->url = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //package/time
      if (reader_r->name() == "time")
      {
        package_ptr->buildTime = reader_r->getAttribute("build").asString();
        // ignoring reader_r->getAttribute("file").asString(); (rpm file timestamp)
        return true;
      }

      // xpath: //package/size
      if (reader_r->name() == "size")
      {
        //! \todo what's 'archive' size of the package (it is neither the rpm file size nor its installed size)
        // reader_r->getAttribute("archive").asString();

        // installed size
        package_ptr->installedSize = str::strtonum<ByteCount::SizeType>( reader_r->getAttribute("installed").asString() );

        // rpm package size
        package_ptr->repositoryLocation.fileSize = str::strtonum<ByteCount::SizeType>( reader_r->getAttribute("package").asString() );

        return true;
      }

      // xpath: //package/location
      if (reader_r->name() == "location")
      {
        package_ptr->repositoryLocation.filePath = reader_r->getAttribute("href").asString();
        return true;
      }

      // xpath: //package/format
      if (reader_r->name() == "format")
      {
        tag(tag_format);
        return true;
      }
    }

    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      // xpath: //package
      if (reader_r->name() == "package")
      {
        // indicate that further processing is required
        // e.g. caller needs to to check for </package> to trigger some action
        return false;
      }
    }

    return true;
  }


  // --------------( consume <format> tag )------------------------------------

  bool FileReaderBase::BaseImpl::consumeFormatNode(
    xml::Reader & reader_r, data::Package_Ptr & package_ptr)
  {
    if (consumeDependency(reader_r, package_ptr->deps))
      // this node has been a dependency, which has been handled by
      // consumeDependency(), so return right away. 
      return true;

    // DBG << "format subtag: " << reader_r->name() << endl;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: //format/rpm:license
      if (reader_r->name() == "rpm:license")
      {
        package_ptr->license = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //format/rpm:vendor
      if (reader_r->name() == "rpm:vendor")
      {
        package_ptr->vendor = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //format/rpm:group
      if (reader_r->name() == "rpm:group")
      {
        package_ptr->group = reader_r.nodeText().asString();
        return true;
      }

      // xpath: //format/rpm:buildhost
      if (reader_r->name() == "rpm:buildhost")
      {
        package_ptr->buildhost = reader_r.nodeText().asString();
        return true;
      }

      //! \todo xpath: //format/rpm:sourcerpm where to store this?
      if (reader_r->name() == "rpm:sourcerpm")
      {
        //package->source = reader_r.nodeText().asString();
        return true;
      }

      //! \todo xpath: //format/rpm:header-range what is this? 
      if (reader_r->name() == "rpm:header-range")
      {
        //reader_r->getAttribute("start").asString(),
        //reader_r->getAttribute("end").asString(),
        return true;
      }

      //! \todo xpath: //format/file (*) figure out where to store this and what's it about (in regard to filelists.xml.gz) 
      if (reader_r->name() == "file")
      {
        // file = reader_r.nodeText().asString();
        // type = reader_r->getAttribute("type").asString();
        return true;
      }

      // xpath: //format/suse:authors/suse:author (+)
      // but tolerating multiplicity (*)
      if (reader_r->name() == "suse:author")
      {
        package_ptr->authors.push_back(reader_r.nodeText().asString());
        return true;
      }

      // xpath: //format/suse:keywords (?)
      // xpath: //format/suse:keywords/suse:keyword (+)
      if (reader_r->name() == "suse:keyword")
      {
        package_ptr->keywords.insert(reader_r.nodeText().asString());
        return true;
      }

      //! \todo xpath: //format/suse:dirsizes (?)

      // xpath: //format/suse:install-only (?)
      if (reader_r->name() == "suse:install-only")
      {
        package_ptr->installOnly = true;
        return true;
      }

      // xpath: //format/suse:license-to-confirm (*)
      //! \todo this is ambiguous - fix the rnc schema
      if (reader_r->name() == "suse:license-to-confirm")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        package_ptr->licenseToConfirm.setText(
            reader_r.nodeText().asString(), locale);
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: //format
      if (reader_r->name() == "format")
      {
        toParentTag();
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool FileReaderBase::BaseImpl::consumeDependency(
    xml::Reader & reader_r, data::Dependencies & deps_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: //format/*/rpm:entry | //format/suse-freshens/suse:entry (+)
      if (reader_r->name() == "rpm:entry" | reader_r->name() == "suse:entry")
      {
        if (!_expect_rpm_entry)
          ZYPP_THROW(ParseException("rpm:entry found when not expected"));

        // read kind of resolvable this entry refers, default to Package
        string kind_str = reader_r->getAttribute("kind").asString();
        Resolvable::Kind kind;
        if (kind_str.empty())
           kind = ResTraits<Package>::kind;
        else
          kind = Resolvable::Kind(kind_str); 

        // Check whether this is actually a prerequires dependency.
        // If so, it will be stored in deps_r as Dep::PREREQUIRES
        // instead of Dep::REQUIRES (a prerequires can appear as part
        // of requires dependencies only).
        bool pre = false;
        if (reader_r->getAttribute("pre").asString() == "1")
        {
          if (_dtype.inSwitch() != Dep::REQUIRES_e)
            ZYPP_THROW(ParseException("pre=\"1\" found for non-requires dependency"));
          pre = true;
        }
/*
        DBG << "got rpm:entry for " << _dtype << ": "
            << reader_r->getAttribute("name").asString()
            << " " << edition << " (" << kind << ")" << endl;
*/

        string version = reader_r->getAttribute("ver").asString();

        if (version.empty())
        {
          // insert unversion dependency into the list
          deps_r[pre ? Dep::PREREQUIRES : _dtype].insert(
            zypp::capability::parse(
              kind, reader_r->getAttribute("name").asString()
            )
          );
        }
        else
        {
          Edition edition(
            version,
            reader_r->getAttribute("rel").asString(),
            reader_r->getAttribute("epoch").asString()
          );

          // insert versioned dependency into the list
          deps_r[pre ? Dep::PREREQUIRES : _dtype].insert(
            zypp::capability::parse(
              kind,
              reader_r->getAttribute("name").asString(),
              Rel(reader_r->getAttribute("flags").asString()),
              edition
            )
          );
        }

        //! \todo check <rpm:entry name="/bin/sh" pre="1">
      }

      // xpath: //format/rpm:provides (?)
      if (reader_r->name() == "rpm:provides")
      {
        _dtype = zypp::Dep::PROVIDES;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:conflicts (?)
      if (reader_r->name() == "rpm:conflicts")
      {
        _dtype = zypp::Dep::CONFLICTS;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:obsoletes (?)
      if (reader_r->name() == "rpm:obsoletes")
      {
        _dtype = zypp::Dep::OBSOLETES;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:requires (?)
      if (reader_r->name() == "rpm:requires")
      {
        _dtype = zypp::Dep::REQUIRES;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:recommends (?)
      if (reader_r->name() == "rpm:recommends")
      {
        _dtype = zypp::Dep::RECOMMENDS;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:enhances (?)
      if (reader_r->name() == "rpm:enhances")
      {
        _dtype = zypp::Dep::ENHANCES;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:supplements (?)
      if (reader_r->name() == "rpm:supplements")
      {
        _dtype = zypp::Dep::SUPPLEMENTS;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/rpm:suggests (?)
      if (reader_r->name() == "rpm:suggests")
      {
        _dtype = zypp::Dep::SUGGESTS;
        _expect_rpm_entry = true;
        return true;
      }
      // xpath: //format/suse:freshens (?)
      if (reader_r->name() == "suse:freshens")
      {
        _dtype = zypp::Dep::FRESHENS;
        _expect_rpm_entry = true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: //format/* (?)
      if (reader_r->name() == "rpm:requires"
          || reader_r->name() == "rpm:provides"
          || reader_r->name() == "rpm:conflicts"
          || reader_r->name() == "rpm:obsoletes"
          || reader_r->name() == "rpm:recommends"
          || reader_r->name() == "rpm:enhances"
          || reader_r->name() == "rpm:supplements"
          || reader_r->name() == "rpm:suggests"
          || reader_r->name() == "suse:freshens")
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
