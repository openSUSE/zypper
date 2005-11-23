/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMParserData.cc
 *
*/



#include <YUMParserData.h>

using namespace std;
namespace zypp {
  namespace parser {
    namespace yum {

      YUMDependency::YUMDependency()
      { }
      
      YUMDependency::YUMDependency(const std::string& kind,
                                   const std::string& name,
                                   const std::string& flags,
                                   const std::string& epoch,
                                   const std::string& ver,
                                   const std::string& rel,
                                   const std::string& pre)
      : kind(kind),
      name(name),
      flags(flags),
      epoch(epoch),
      ver(ver),
      rel(rel),
      pre(pre)
      { };
      
      YUMDirSize::YUMDirSize()
      { }
      
      YUMDirSize::YUMDirSize(const std::string& path,
                             const std::string& sizeKByte,
                             const std::string& fileCount)
      : path(path), sizeKByte(sizeKByte), fileCount(fileCount)
      { }
      
      YUMRepomdData::YUMRepomdData()
      { }
      
      YUMPrimaryData::YUMPrimaryData()
      { }
      
      FileData::FileData()
      { }
      
      YUMPatchData::YUMPatchData()
      { }
      
      FileData::FileData(const std::string &name,
                         const std::string &type)
      : name(name), type(type)
      { }
      
      
      YUMGroupData::YUMGroupData()
      { }
      
      MultiLang::MultiLang()
      { }
      
      MultiLang::MultiLang(const std::string& language,
                           const std::string& text)
      : language(language), text(text)
      { }
      
      
      MetaPkg::MetaPkg()
      { }
      
      MetaPkg::MetaPkg(const std::string& type,
                       const std::string& name)
      : type(type), name(name)
      { }
      
      PackageReq::PackageReq()
      { }
      
      PackageReq::PackageReq(const std::string& type,
                             const std::string& epoch,
                             const std::string& ver,
                             const std::string& rel,
                             const std::string& name)
      : type(type), epoch(epoch), ver(ver), rel(rel), name(name)
      { }
      
      ChangelogEntry::ChangelogEntry()
      { }
      
      ChangelogEntry::ChangelogEntry(const std::string& author,
                                     const std::string& date,
                                     const std::string& entry)
      : author(author), date(date), entry(entry)
      { }
      
                      
      YUMFileListData::YUMFileListData()
      { }
      
      YUMOtherData::YUMOtherData()
      { }
      
      
      /* Define pointer classes */
      
      IMPL_PTR_TYPE(YUMRepomdData);
      IMPL_PTR_TYPE(YUMPrimaryData);
      IMPL_PTR_TYPE(YUMGroupData);
      IMPL_PTR_TYPE(YUMFileListData);
      IMPL_PTR_TYPE(YUMOtherData);
      IMPL_PTR_TYPE(YUMPatchData);
      IMPL_PTR_TYPE(YUMPatchPackage);
      IMPL_PTR_TYPE(YUMPatchScript);
      IMPL_PTR_TYPE(YUMPatchMessage);
      
      /* output operators */
      
      namespace {
        /**
         * @short Generic stream output for lists of Ptrs
         * @param out the ostream where the output goes to
         * @param aList the list to output
         * @return is out
         */
        template<class T>
        ostream& operator<<(ostream &out, const list<T>& aList)
        {
          typedef typename list<T>::const_iterator IterType;
          for (IterType iter = aList.begin();
              iter != aList.end();
              ++iter) {
                if (iter != aList.begin())
                  out << endl;
                ::operator<<(out,*iter);
              }
          return out;
        }
      }
      
      /**
       * Join a list of strings into a single string
       * @param aList the list of strings
       * @param joiner what to put between the list elements
       * @return the joined string
       */
      string join(const list<string>& aList,
                  const string& joiner)
      {
        string res;
        for (list<string>::const_iterator iter = aList.begin();
            iter != aList.end();
            ++ iter) {
              if (iter != aList.begin())
                res += joiner;
              res += *iter;
            }
        return res;
      }

    } // namespace yum
  } // namespace parser
} // namespace zypp

using namespace zypp::parser::yum;
  
ostream& operator<<(ostream &out, const YUMDependency& data)
{
  if (! data.kind.empty())
    out << "[" << data.kind << "] ";
  out << data.name << " " << data.flags << " ";
  if (! data.epoch.empty())
    out << data.epoch << "-";
  out << data.ver << "-" << data.rel ;
  if (! data.pre.empty() && data.pre != "0")
    out << " (pre=" << data.pre << ")";
  return out;
}
  
ostream& operator<<(ostream &out, const YUMDirSize& data)
{
  out << data.path
    << ": " << data.sizeKByte << " kByte, "
    << data.fileCount << " files";
  return out;
}
    
ostream& operator<<(ostream &out, const FileData& data)
{
  out << data.name;
  if (! data.type.empty()) {
    out << ": " << data.type;
  }
  return out;
}

ostream& operator<<(ostream &out, const MultiLang& data)
{
  if (!data.language.empty())
    out << "[" << data.language << "] ";
  out << data.text;
  return out;
}

ostream& operator<<(ostream &out, const MetaPkg& data)
{
  out << "type: " << data.type
    << ", name: " << data.name;
  return out;
}

ostream& operator<<(ostream &out, const PackageReq& data)
{
  out << "[" << data.type << "] "
    << data.name
    << " " << data.epoch
    << "-" << data.ver
    << "-" << data.rel;
  return out;
}

ostream& operator<<(ostream &out, const ChangelogEntry& data)
{
  out << data.date
    << " - " << data.author << endl
    << data.entry;
  return out;
}

ostream& operator<<(ostream &out, const YUMRepomdData& data)
{
  out << "Repomd Data: " << endl
    << "  type: '" << data.type << "'" << endl
    << "  location: '" << data.location << "'" <<endl
    << "  checksumType: '" << data.checksumType << "'" << endl
    << "  checksum: '" << data.checksum << "'" << endl
    << "  timestamp: '" << data.timestamp << "'" << endl
    << "  openChecksumType: '" << data.openChecksumType << "'" << endl
    << "  openChecksum: '" << data.openChecksum << "'" << endl;
  return out;
}

ostream& operator<<(ostream &out, const YUMPrimaryData& data)
{
  out << "-------------------------------------------------" << endl
    << "Primary Data: " << endl
    << "name: '" << data.name << "'" << endl
    << "type: '" << data.type << "'" << endl
    << " arch: '" << data.arch << "'" << endl
    << " ver: '" << data.ver << "'" << endl
    << "checksumType: '" << data.checksumType << "'" << endl
    << "checksumPkgid: '" << data.checksumPkgid << "'" << endl
    << "checksum: '" << data.checksum << "'" << endl
    << "summary: '" << data.summary << "'" << endl
    << "description: '" << data.description << "'" << endl
    << "packager: '" << data.packager << "'" << endl
    << "url: '" << data.url << "'" << endl
    << "timeFile: '" << data.timeFile << "'" << endl
    << "timeBuild: '" << data.timeBuild << "'" << endl
    << "sizePackage: '" << data.sizePackage << "'" << endl
    << "sizeInstalled: '" << data.sizeInstalled << "'" << endl
    << "sizeArchive: '" << data.sizeArchive << "'" << endl
    << "location: '" << data.location << "'" << endl
    << "license: '" << data.license << "'" << endl
    << "vendor: '" << data.vendor << "'" << endl
    << "group: '" << data.group << "'" << endl
    << "buildhost: '" << data.buildhost << "'" << endl
    << "sourcerpm: '" << data.sourcerpm << "'" << endl
    << "headerStart: '" << data.headerStart << "'" << endl
    << "headerEnd: '" << data.headerEnd << "'" << endl
    << "provides:" << endl
    << data.provides << endl
    << "conflicts:" << endl
    << data.conflicts << endl
    << "obsoletes:" << endl
    << data.obsoletes << endl
    << "requires:" << endl
    << data.requires << endl
    << "files:" << endl
    << data.files << endl
    << "authors: " << join(data.authors,", ") << endl
    << "keywords: " << join(data.keywords,", ") << endl
    << "media: " << data.media << endl
    << "dirsizes: " << endl
    << data.dirSizes << endl
    << "freshen: " << endl
    << data.freshen << endl
    << "install-only: '" << data.installOnly << "'" << endl;
  return out;
}

ostream& operator<<(ostream &out, const YUMGroupData& data)
{
  out << "-------------------------------------------------" << endl
    << "Group Data: " << endl
    << "group-id: '" << data.groupId << "'" << endl
    << "name:" << endl
    << data.name << endl
    << "default: '" << data.default_  << "'" << endl
    << "user-visible: '" << data.userVisible  << "'" << endl
    << "description:" << endl
    << data.description << endl
    << "grouplist:" << endl
    << data.grouplist << endl
    << "packageList:" << endl
    << data.packageList << endl;
  return out;
}

ostream& operator<<(ostream &out, const YUMFileListData& data)
{
  out << "-------------------------------------------------" << endl
    << "File List: " << endl
    << "pkgid: " << data.pkgId << endl
    << "package: " << data.name << " "
    << data.epoch << "-" << data.ver << "-" << data.rel << endl
    << "files:" << endl
    << data.files << endl;
  return out;
}

ostream& operator<<(ostream& out, const YUMOtherData& data)
{
  out << "-------------------------------------------------" << endl
    << "Other: " << endl
    << "pkgid: " << data.pkgId
    << "package: " << data.name << " "
    << data.epoch << "-" << data.ver << "-" << data.rel << endl
    << "Changelog:" << endl
    << data.changelog << endl;
  return out;
}

ostream& operator<<(ostream &out, const YUMPatchData& data)
{
  out << "-------------------------------------------------" << endl
    << "Patch Data: " << endl
    << "  patch ID: " << data.patchId << endl
    << "  timestamp: '" << data.timestamp << "'" << endl
    << "  engine: '" << data.engine << "'" << endl
    << "  name: " << data.name << endl
    << "  summary: " << data.summary << endl
    << "  description: " << data.description << endl
    << "  epoch: " << data.epoch << endl
    << "  version: " << data.ver << endl
    << "  release: " << data.rel << endl
    << "  provides: " << data.provides << endl
    << "  conflicts: " << data.conflicts << endl
    << "  obsoletes: " << data.obsoletes << endl
    << "  freshen: " << data.freshen << endl
    << "  requires: " << data.requires << endl
    << "  category: " << data.category << endl
    << "  reboot needed: " << data.rebootNeeded << endl
    << "  affects package manager: " << data.packageManager << endl
    << "  update script: " << data.updateScript << endl
    << "  atoms:" << endl
    << data.atoms;
  return out;
}

std::ostream& operator<<(std::ostream& out, const shared_ptr<YUMPatchAtom> data)
{
  out << "Atom data" << endl;
  switch (data->atomType())
  {
    case YUMPatchAtom::Package:
      out << "  atom type: " << "package" << endl
        << *dynamic_pointer_cast<YUMPatchPackage>(data);
      break;
    case YUMPatchAtom::Message:
      out << "  atom type: " << "message" << endl
        << *dynamic_pointer_cast<YUMPatchMessage>(data);
      break;
    case YUMPatchAtom::Script:
      out << "  atom type: " << "script" << endl
        << *dynamic_pointer_cast<YUMPatchScript>(data);
      break;
    default:
      out << "Unknown atom type" << endl;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const YUMPatchMessage& data)
{
  out << "Message Data: " << endl
    << "  name: " << data.name << endl
    << "  type: " << data.type << endl
    << "  epoch: " << data.epoch << endl
    << "  version: " << data.ver << endl
    << "  release: " << data.rel << endl
    << "  provides: " << data.provides << endl
    << "  conflicts: " << data.conflicts << endl
    << "  obsoletes: " << data.obsoletes << endl
    << "  freshen: " << data.freshen << endl
    << "  requires: " << data.requires << endl
    << "  text: " << data.text << endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, const YUMPatchScript& data)
{
  out << "Script Data: " << endl
    << "  name: " << data.name << endl
    << "  epoch: " << data.epoch << endl
    << "  version: " << data.ver << endl
    << "  release: " << data.rel << endl
    << "  provides: " << data.provides << endl
    << "  conflicts: " << data.conflicts << endl
    << "  obsoletes: " << data.obsoletes << endl
    << "  freshen: " << data.freshen << endl
    << "  requires: " << data.requires << endl
    << "  do script: " << data.do_script << endl
    << "  undo script: " << data.undo_script << endl;
  return out;
}

std::ostream& operator<<(std::ostream& out, const YUMPatchPackage& data)
{
  out << "Package Data: " << endl
    << "  name: '" << data.name << "'" << endl
    << "  type: '" << data.type << "'" << endl
    << "   arch: '" << data.arch << "'" << endl
    << "   ver: '" << data.ver << "'" << endl
    << "  checksumType: '" << data.checksumType << "'" << endl
    << "  checksumPkgid: '" << data.checksumPkgid << "'" << endl
    << "  checksum: '" << data.checksum << "'" << endl
    << "  summary: '" << data.summary << "'" << endl
    << "  description: '" << data.description << "'" << endl
    << "  packager: '" << data.packager << "'" << endl
    << "  url: '" << data.url << "'" << endl
    << "  timeFile: '" << data.timeFile << "'" << endl
    << "  timeBuild: '" << data.timeBuild << "'" << endl
    << "  sizePackage: '" << data.sizePackage << "'" << endl
    << "  sizeInstalled: '" << data.sizeInstalled << "'" << endl
    << "  sizeArchive: '" << data.sizeArchive << "'" << endl
    << "  location: '" << data.location << "'" << endl
    << "  license: '" << data.license << "'" << endl
    << "  vendor: '" << data.vendor << "'" << endl
    << "  group: '" << data.group << "'" << endl
    << "  buildhost: '" << data.buildhost << "'" << endl
    << "  sourcerpm: '" << data.sourcerpm << "'" << endl
    << "  headerStart: '" << data.headerStart << "'" << endl
    << "  headerEnd: '" << data.headerEnd << "'" << endl
    << "  provides:" << endl
    << data.provides << endl
    << "  conflicts:" << endl
    << data.conflicts << endl
    << "  obsoletes:" << endl
    << data.obsoletes << endl
    << "  requires:" << endl
    << data.requires << endl
    << "  files:" << endl
    << data.files << endl
    << "  authors: " << join(data.authors,", ") << endl
    << "  keywords: " << join(data.keywords,", ") << endl
    << "  media: " << data.media << endl
    << "  dirsizes: " << endl
    << data.dirSizes << endl
    << "  freshen: " << endl
    << data.freshen << endl
    << "  install-only: '" << data.installOnly << "'" << endl
    << "  files:" << endl
    << data.files << endl
    << "  Changelog:" << endl
    << data.changelog << endl
    << "  Plain RPM:" << endl
    << "    arch: " << data.plainRpm.arch << endl
    << "    filename: " << data.plainRpm.filename << endl
    << "    download size: " << data.plainRpm.downloadsize << endl
    << "    MD5: " << data.plainRpm.md5sum << endl
    << "    build time: " << data.plainRpm.buildtime << endl
    << "  Patch RPM:" << endl
    << "    arch: " << data.patchRpm.arch << endl
    << "    filename: " << data.patchRpm.filename << endl
    << "    download size: " << data.patchRpm.downloadsize << endl
    << "    MD5: " << data.patchRpm.md5sum << endl
    << "    build time: " << data.patchRpm.buildtime << endl
    << data.patchRpm.baseVersions
    << "  Delta RPM:" << endl
    << "    arch: " << data.deltaRpm.arch << endl
    << "    filename: " << data.deltaRpm.filename << endl
    << "    download size: " << data.deltaRpm.downloadsize << endl
    << "    MD5: " << data.deltaRpm.md5sum << endl
    << "    build time: " << data.deltaRpm.buildtime << endl
    << data.deltaRpm.baseVersion;
  return out;
}

std::ostream& operator<<(std::ostream& out, const YUMBaseVersion& data)
{
  out << "    Base version:" << endl
    << "      epoch: " << data.epoch << endl
    << "      version: " << data.ver << endl
    << "      release: " << data.rel << endl
    << "      MD5: " << data.md5sum << endl
    << "      build time: " << data.buildtime << endl
    << "      source info: " << data.source_info << endl;
  return out;
}
