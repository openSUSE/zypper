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
#include "zypp/Pathname.h"
#include "zypp/TranslatedText.h"
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
      DEFINE_PTR_TYPE( YUMPatternData );
      DEFINE_PTR_TYPE( YUMFileListData );
      DEFINE_PTR_TYPE( YUMOtherData );
      DEFINE_PTR_TYPE( YUMPatchData );
      DEFINE_PTR_TYPE( YUMPatchesData );
      DEFINE_PTR_TYPE( YUMProductData );
      DEFINE_PTR_TYPE( YUMPatchPackage );
      DEFINE_PTR_TYPE( YUMPatchScript );
      DEFINE_PTR_TYPE( YUMPatchMessage );

      /**
      * @short Holds dependency data
      */
      class YUMDependency {
      public:
        YUMDependency();
        YUMDependency(const std::string& kind,
                      const std::string& name,
                      const std::string& flags,
                      const std::string& epoch,
                      const std::string& ver,
                      const std::string& rel,
                      const std::string& pre );
        YUMDependency(const std::string& kind,
                      const std::string& encoded );
        bool isEncoded() const;
        std::string kind;
        std::string name;
        std::string flags;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::string pre;
        // in case we only store the encoded string
        std::string encoded;
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

      class YUMObjectData : public base::ReferenceCounted, private base::NonCopyable {
      public:

        std::string name;
        std::string epoch;
        std::string ver;
        std::string rel;
        std::list<YUMDependency> provides;
        std::list<YUMDependency> conflicts;
        std::list<YUMDependency> obsoletes;
        std::list<YUMDependency> freshen;
        std::list<YUMDependency> requires;
        std::list<YUMDependency> recommends;
        std::list<YUMDependency> suggests;
        std::list<YUMDependency> enhances;
      };

      class YUMPatchAtom : public YUMObjectData {
      public:
	enum AtomType { Package, Script, Message };
        virtual AtomType atomType() = 0;
      };

      class PlainRpm {
      public:
	std::string arch;
	std::string filename;
	std::string downloadsize;
	std::string md5sum;
	std::string buildtime;
      };

      class PatchRpm {
      public:
	std::string arch;
	std::string filename;
	std::string downloadsize;
	std::string md5sum;
	std::string buildtime;
	std::list<YUMBaseVersion> baseVersions;
      };

      class DeltaRpm {
      public:
	std::string arch;
	std::string filename;
	std::string downloadsize;
	std::string md5sum;
	std::string buildtime;
	YUMBaseVersion baseVersion;
      };


      class YUMPatchPackage : public YUMPatchAtom {
      public:
        YUMPatchPackage() {};
        virtual AtomType atomType() { return Package; };
        // data for primary
        std::string arch;
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
	std::list<PlainRpm> plainRpms;
	std::list<PatchRpm> patchRpms;
	std::list<DeltaRpm> deltaRpms;
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
        TranslatedText text;
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
      class YUMPrimaryData : public YUMObjectData {
      public:
        YUMPrimaryData();
        std::string type;
        std::string arch;
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
        Pathname location;
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
      };

      /**
      * @short Describes the groups in a YUM repository
      **/

      class YUMGroupData : public base::ReferenceCounted, private base::NonCopyable {
      public:

        YUMGroupData();
        std::string groupId;
        TranslatedText name;
        std::string default_;
        std::string userVisible;
        TranslatedText description;
        std::list<MetaPkg> grouplist;
        std::list<PackageReq> packageList;
      };

      /**
      * @short Describes the patterns in a YUM repository
      **/

      class YUMPatternData : public YUMObjectData {
      public:

        YUMPatternData();
        std::string name;
        TranslatedText summary;
        std::string default_;
        std::string userVisible;
        TranslatedText description;
        TranslatedText category;
        std::string icon;
        std::string script;
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

      class YUMPatchData : public YUMObjectData {
      public:
        YUMPatchData();
        ~YUMPatchData() {

        }

        std::string patchId;
        std::string timestamp;
        std::string engine;
        TranslatedText summary;
        TranslatedText description;
        std::string category;
        bool rebootNeeded;
        bool packageManager;
        std::string updateScript;
        std::list<shared_ptr<YUMPatchAtom> > atoms;
      };

      class YUMPatchesData : public base::ReferenceCounted, private base::NonCopyable {
      public:
	YUMPatchesData() {};
	~YUMPatchesData() {};

	std::string location;
	std::string id;
        std::string checksumType;
        std::string checksum;
      };

      class YUMProductData : public YUMObjectData {
      public:
        YUMProductData() {};
         ~YUMProductData() {};

        std::string type;
        std::string vendor;
        std::string name; 
        TranslatedText displayname;
        TranslatedText description;
        // those are suse specific tags
        std::string releasenotesurl;
      };

      /* Easy output */
      std::ostream& operator<<(std::ostream &out, const YUMDependency& data);
      std::ostream& operator<<(std::ostream &out, const YUMDirSize& data);
      std::ostream& operator<<(std::ostream &out, const YUMRepomdData& data);
      std::ostream& operator<<(std::ostream &out, const FileData& data);
      std::ostream& operator<<(std::ostream &out, const MetaPkg& data);
      std::ostream& operator<<(std::ostream &out, const PackageReq& data);
      std::ostream& operator<<(std::ostream &out, const ChangelogEntry& data);
      std::ostream& operator<<(std::ostream &out, const YUMRepomdData& data);
      std::ostream& operator<<(std::ostream &out, const YUMPrimaryData& data);
      std::ostream& operator<<(std::ostream &out, const YUMGroupData& data);
      std::ostream& operator<<(std::ostream &out, const YUMPatternData& data);
      std::ostream& operator<<(std::ostream &out, const YUMFileListData& data);
      std::ostream& operator<<(std::ostream& out, const YUMOtherData& data);
      std::ostream& operator<<(std::ostream& out, const YUMPatchData& data);
      std::ostream& operator<<(std::ostream& out, const YUMPatchesData& data);
      std::ostream& operator<<(std::ostream& out, const YUMProductData& data);
      std::ostream& operator<<(std::ostream& out, const zypp::shared_ptr<YUMPatchAtom> data);
      std::ostream& operator<<(std::ostream& out, const YUMPatchMessage& data);
      std::ostream& operator<<(std::ostream& out, const YUMPatchScript& data);
      std::ostream& operator<<(std::ostream& out, const YUMPatchPackage& data);
      std::ostream& operator<<(std::ostream& out, const YUMBaseVersion& data);
      std::ostream& operator<<(std::ostream& out, const PlainRpm& data);
      std::ostream& operator<<(std::ostream& out, const PatchRpm& data);
      std::ostream& operator<<(std::ostream& out, const DeltaRpm& data);

    } // namespace yum
  } // namespace parser
} // namespace zypp






#endif
