/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMParserData.h
 *
 * 
 * Purpose: Declares the various YUMData classes, which are rather dumb
 *          structure-like classes that hold the data of specific YUM
 *          repository files. The parsers (YUM...Parser) create these objects,
 *          and the YUM installation source use them to build more
 *          sophisticated objects.
/-*/

#ifndef YUMParserData_h
#define YUMParserData_h

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include <string>
#include <list>
#include <iostream>
#include <zypp/base/PtrTypes.h>

using namespace zypp::base;


namespace zypp {
  namespace parser {
    namespace yum {

      DEFINE_PTR_TYPE( YUMRepomdData );
      DEFINE_PTR_TYPE( YUMPrimaryData );
      DEFINE_PTR_TYPE( YUMGroupData );
      DEFINE_PTR_TYPE( YUMFileListData );
      DEFINE_PTR_TYPE( YUMOtherData );
      DEFINE_PTR_TYPE( YUMPatchData );
      DEFINE_PTR_TYPE( YUMPatchPackage );
      DEFINE_PTR_TYPE( YUMPatchScript );
      DEFINE_PTR_TYPE( YUMPatchMessage );
    
      /**
      * @short Holds dependency data
      */
      class YUMDependency {
      public:
        YUMDependency();
        YUMDependency(const std::string& name,
                      const std::string& flags,
                      const std::string& epoch,
                      const std::string& ver,
                      const std::string& rel,
                      const std::string& pre);
        std::string name;
        std::string flags;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string pre;
      };
    
    
      /**
      * @short Holds data about how much space will be needed per directory
      **/
      class YUMDirSize {
      public:
        YUMDirSize();
        YUMDirSize(const std::string& path,
                  const std::string& size,
                  const std::string& fileCount);
        const std::string path;
        const std::string sizeKByte;
        const std::string fileCount;
      };
    
      /**
       * @short Holds Data about file and file type
       *  (directory, plain) within other YUM data
       **/
      class FileData {
      public:
        std::string name;
        std::string type;
        FileData();
        FileData(const std::string &name,
                 const std::string &type);
      };
    
      /**
       * @short A Multi-language text
       * (usually you have a list<MultiLang>)
       **/
      class MultiLang {
      public:
        MultiLang();
        MultiLang(const std::string& langugage,
                  const std::string& text);
        std::string language;
        std::string text;
      };
    
      /**
       * @short Defines "meta packages" that are in YUMGroupData
       * FIXME: I'm not certain what this is ;-)
       **/
      class MetaPkg {
      public:
        MetaPkg();
        MetaPkg(const std::string& type,
                const std::string& name);
        std::string type;
        std::string name;
      };
    
      /**
       * @short A Package reference, e.g. within YUMGroupData
       * this is without architecture.
       **/
      class PackageReq {
      public:
        PackageReq();
        PackageReq(const std::string& type,
                  const std::string& epoch,
                  const std::string& ver,
                  const std::string& rel,
                  const std::string& name);
        std::string type;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string name;
      };
    
      /**
      * @short A single changelog entry
      **/
      class ChangelogEntry {
      public:
        ChangelogEntry();
        ChangelogEntry(const std::string& author,
                      const std::string& date,
                      const std::string& entry);
        std::string author;
        std::string date;
        std::string entry;
      };
    
      class YUMBaseVersion {
      public:
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string md5sum;
        std::string buildtime;
        std::string source_info;
      };
    
      class YUMPatchAtom : public base::ReferenceCounted, private base::NonCopyable {
      public:

	enum AtomType { Package, Script, Message };

        virtual AtomType atomType() = 0;
        std::string name;
        std::string type;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string arch;
        std::list<YUMDependency> provides;
        std::list<YUMDependency> conflicts;
        std::list<YUMDependency> obsoletes;
        std::list<YUMDependency> freshen;
        std::list<YUMDependency> requires;
      };

      class YUMPatchPackage : public YUMPatchAtom {
      public:
        YUMPatchPackage() {};
        virtual AtomType atomType() { return Package; };
        // data for primary
        std::string type;
        std::string checksumType;
        std::string checksumPkgid;
        std::string checksum;
        std::string summary;
        std::string description;
        std::string packager;
        std::string url;
        std::string timeFile;
        std::string timeBuild;
        std::string sizePackage;
        std::string sizeInstalled;
        std::string sizeArchive;
        std::string location;
        std::string license;
        std::string vendor;
        std::string group;
        std::string buildhost;
        std::string sourcerpm;
        std::string headerStart;
        std::string headerEnd;
        std::list<FileData> files;
        // SuSE specific data
        std::list<std::string> authors;
        std::list<std::string> keywords;
        std::string  media;
        std::list<YUMDirSize> dirSizes;
        bool installOnly;
        // Change Log
        std::list<ChangelogEntry> changelog;
        // Package Files
        struct {
          std::string arch;
          std::string filename;
          std::string downloadsize;
          std::string md5sum;
          std::string buildtime;
        } plainRpm;
        struct {
          std::string arch;
          std::string filename;
          std::string downloadsize;
          std::string md5sum;
          std::string buildtime;
          std::list<YUMBaseVersion> baseVersions;
        } patchRpm;
        struct {
          std::string arch;
          std::string filename;
          std::string downloadsize;
          std::string md5sum;
          std::string buildtime;
          YUMBaseVersion baseVersion;
        } deltaRpm;
      };
    
      class YUMPatchScript : public YUMPatchAtom {
      public:
        YUMPatchScript() {};
        virtual AtomType atomType() { return Script; };
        std::string do_script;
        std::string undo_script;
      };
    
      class YUMPatchMessage : public YUMPatchAtom {
      public:
        YUMPatchMessage() {};
        virtual AtomType atomType() { return Message; };
        std::string type;
        std::string text;
      };
    
   
      /**
       * @short Holds the metadata about a YUM repository
       **/
      class YUMRepomdData : public base::ReferenceCounted, private base::NonCopyable {
      public:
        YUMRepomdData();
        std::string type;
        std::string location;
        std::string checksumType;
        std::string checksum;
        std::string timestamp;
        std::string openChecksumType;
        std::string openChecksum;
      };
    
      /**
       * @short Describes a package in a YUM repository
       **/
      class YUMPrimaryData : public base::ReferenceCounted, private base::NonCopyable {
      public:
        YUMPrimaryData();
        std::string type;
        std::string name;
        std::string arch;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string checksumType;
        std::string checksumPkgid;
        std::string checksum;
        std::string summary;
        std::string description;
        std::string packager;
        std::string url;
        std::string timeFile;
        std::string timeBuild;
        std::string sizePackage;
        std::string sizeInstalled;
        std::string sizeArchive;
        std::string location;
        std::string license;
        std::string vendor;
        std::string group;
        std::string buildhost;
        std::string sourcerpm;
        std::string headerStart;
        std::string headerEnd;
        std::list<YUMDependency> provides;
        std::list<YUMDependency> conflicts;
        std::list<YUMDependency> obsoletes;
        std::list<YUMDependency> requires;
        std::list<FileData> files;
    
        // SuSE specific data
        std::list<std::string> authors;
        std::list<std::string> keywords;
        std::string  media;
        std::list<YUMDirSize> dirSizes;
        std::list<YUMDependency> freshen;
        bool installOnly;
      };
    
      /**
      * @short Describes the groups in a YUM repository
      **/
    
      class YUMGroupData : public base::ReferenceCounted, private base::NonCopyable {
      public:
    
        YUMGroupData();
        std::string groupId;
        std::list<MultiLang> name;
        std::string default_;
        std::string userVisible;
        std::list<MultiLang> description;
        std::list<MetaPkg> grouplist;
        std::list<PackageReq> packageList;
      };
    
      /**
       * @short Contains the file list for a YUM package.
       **/
      class YUMFileListData : public base::ReferenceCounted, private base::NonCopyable {
      public:
    
        YUMFileListData();
    
        std::string pkgId;
        std::string name;
        std::string arch;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::list<FileData> files;
      };
    
      /**
      * @short Data from other.mxl, i.e., changelogs
      **/
      class YUMOtherData : public base::ReferenceCounted, private base::NonCopyable {
      public:
        YUMOtherData();
        std::string pkgId;
        std::string name;
        std::string arch;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::list<ChangelogEntry> changelog;
      };
    
    /* ** YUMPatchData not yet finalized **/
    
      class YUMPatchData : public base::ReferenceCounted, private base::NonCopyable {
      public:
        YUMPatchData();
        ~YUMPatchData() {
    
        }
    
        std::string patchId;
        std::string timestamp;
        std::string engine;
        std::string name;
        MultiLang summary;
        MultiLang description;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::list<YUMDependency> provides;
        std::list<YUMDependency> conflicts;
        std::list<YUMDependency> obsoletes;
        std::list<YUMDependency> freshen;
        std::list<YUMDependency> requires;
        std::string category;
        bool rebootNeeded;
        bool packageManager;
        std::string updateScript;
        // FIXME this copying of structures is inefective
        std::list<shared_ptr<YUMPatchAtom> > atoms;
      };


    } // namespace yum
  } // namespace parser
} // namespace zypp


  /* Easy output */

std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMDependency& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMDirSize& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMRepomdData& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::FileData& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::MultiLang& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::MetaPkg& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::PackageReq& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::ChangelogEntry& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMRepomdData& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMPrimaryData& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMGroupData& data);
std::ostream& operator<<(std::ostream &out, const zypp::parser::yum::YUMFileListData& data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMOtherData& data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMPatchData& data);
std::ostream& operator<<(std::ostream& out, const shared_ptr<zypp::parser::yum::YUMPatchAtom> data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMPatchMessage& data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMPatchScript& data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMPatchPackage& data);
std::ostream& operator<<(std::ostream& out, const zypp::parser::yum::YUMBaseVersion& data);




#endif
