/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/PackageImplIf.h
 *
*/
#ifndef ZYPP_DETAIL_PACKAGEIMPLIF_H
#define ZYPP_DETAIL_PACKAGEIMPLIF_H

#include <set>

#include "zypp/detail/ResObjectImplIf.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/Changelog.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Package;

  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PackageImplIf
    //
    /** Abstact Package implementation interface.
    */
    class PackageImplIf : public ResObjectImplIf
    {
    public:
      typedef Package ResType;
      class CheckSum
      {
      public:
        CheckSum(const std::string & type, const std::string & checksum)
	: _type(type)
	, _checksum(checksum)
	{}
        std::string type() { return _type; }
        std::string checksum() { return _checksum; }
      private:
        std::string _type;
        std::string _checksum;
      };
      class BaseVersion
      {
      public:
	BaseVersion(const Edition & edition,
	            const CheckSum & checksum,
		    const Date & buildtime)
	: _edition(edition)
	, _checksum(checksum)
	, _buildtime(buildtime)
        {}
	Edition edition() const { return _edition; }
	CheckSum checksum() const { return _checksum; }
	Date buildtime() const { return _buildtime; }
      private:
	Edition _edition;
        PackageImplIf::CheckSum _checksum;
        Date _buildtime;
      };
      class DeltaRpm
      {
      public:
	DeltaRpm(const Arch & arch,
	         const std::string & filename, 
		 const ByteCount & downloadsize,
		 const CheckSum & checksum,
		 const Date & buildtime,
		 const BaseVersion & base_version)
	: _arch(arch)
	, _filename(filename)
	, _downloadsize(downloadsize)
	, _checksum(checksum)
	, _buildtime(buildtime)
	, _base_version(base_version)
	{}
	Arch arch() { return _arch; }
	std::string filename() { return _filename; }
	ByteCount downloadsize() { return _downloadsize; }
	CheckSum checksum() { return _checksum; }
	Date buildtime() { return _buildtime; }
	BaseVersion baseVersion() { return _base_version; }
      private:
	Arch _arch;
	std::string _filename;
	ByteCount _downloadsize;
	CheckSum _checksum;
	Date _buildtime;
	BaseVersion _base_version;
      };
      class PatchRpm
      {
      public:
	PatchRpm(const Arch & arch,
	         const std::string & filename, 
		 const ByteCount & downloadsize,
		 const CheckSum & checksum,
		 const Date & buildtime,
		 const std::list<BaseVersion> & base_versions)
	: _arch(arch)
	, _filename(filename)
	, _downloadsize(downloadsize)
	, _checksum(checksum)
	, _buildtime(buildtime)
	, _base_versions(base_versions)
	{}
	Arch arch() { return _arch; }
	std::string filename() { return _filename; }
	ByteCount downloadsize() { return _downloadsize; }
	CheckSum checksum() { return _checksum; }
	Date buildtime() { return _buildtime; }
	std::list<BaseVersion> baseVersions() { return _base_versions; }
      private:
	Arch _arch;
	std::string _filename;
	ByteCount _downloadsize;
	CheckSum _checksum;
	Date _buildtime;
	std::list<BaseVersion> _base_versions;
      };

      class DiskUsage {
      public:
	/**
	* @short Holds data about how much space will be needed per directory
	**/
	struct Entry {
	  Entry() : _size(0), _files(0) {};
	  Entry(const std::string& path_r,
                  const unsigned size_r = 0,
                  const unsigned files_r = 0)
	  : path(path_r), _size(size_r), _files(files_r)
	  {}
          const std::string path;
          mutable unsigned _size;
          mutable unsigned _files;
	  friend std::ostream & operator<<( std::ostream & str, const Entry & obj );
          /**
           * Test for equality based on directory name.
           **/
          bool operator==( const Entry & rhs ) const {
            return  path == rhs.path;
          }
          /**
           * Order based on directory name.
           **/
          bool operator<( const Entry & rhs ) const {
            return  path < rhs.path;
          }
          /**
           * Return true if this entry denotes a directory equal to or below rhs._dirname.
           **/
          bool isBelow( const Entry & rhs ) const {
            // whether _dirname has prefix rhs._dirname
            return( path.compare( 0, rhs.path.size(), rhs.path ) == 0 );
          }
          /**
           * Return true if this entry denotes a directory equal to or below dirname_r.
           **/
          bool isBelow( const std::string & dirname_r ) const {
            return  isBelow( Entry( dirname_r ) );
          }
    
          /**
           * 
           **/
          const Entry & operator=( const Entry & rhs ) const {
	    return rhs;
          }
          /**
           * Numerical operation based on size and files values.
           **/
          const Entry & operator+=( const Entry & rhs ) const {
            _size  += rhs._size;
            _files += rhs._files;
            return *this;
          }
          /**
           * Numerical operation based on size and files values.
           **/
          const Entry & operator-=( const Entry & rhs ) const {
            _size  -= rhs._size;
            _files -= rhs._files;
            return *this;
          }
        };
      private:
	typedef std::set<Entry> EntrySet;
	EntrySet _dirs;
      public:

	DiskUsage() {};
       /**
         * Add an entry. If already present, sum up the new entries size and files value.
         **/
        void add( const Entry & newent_r ) {
	  std::pair<EntrySet::iterator,bool> res = _dirs.insert( newent_r );
	  if ( !res.second ) {
	    *res.first += newent_r;
	  }
	}
       /**
         * Add an entry. If already present, sum up the new entries size and files value.
         **/
        void add( const std::string & dirname_r, const unsigned & size_r = 0, const unsigned & files_r = 0 ) {
          add( Entry( dirname_r, size_r, files_r ) );
	}
        /**
         * Number of entries
         **/
        unsigned size() const { return _dirs.size(); }
        /**
         * Clear EntrySet
         **/
        void clear() { _dirs.clear(); }
        /**
         * Sum up any entries for dirname_r and its descendants and remove them
         * on the fly. Return the result.
         **/
        Entry extract( const std::string & dirname_r );

      public:

        typedef EntrySet::iterator               iterator;
        typedef EntrySet::reverse_iterator       reverse_iterator;
     
        /**
         * Forward iterator pointing to the first entry (if any)
         **/
        iterator begin() { return _dirs.begin(); }
        /**
         * Forward iterator pointing behind the last entry.
         **/
        iterator end() { return _dirs.end(); }
        /**
         * Reverse iterator pointing to the last entry (if any)
         **/
        reverse_iterator rbegin() { return _dirs.rbegin(); }
        /**
         * Reverse iterator pointing before the first entry.
         **/
        reverse_iterator rend() { return _dirs.rend(); }
     
        typedef EntrySet::const_iterator         const_iterator;
        typedef EntrySet::const_reverse_iterator const_reverse_iterator;
     
        /**
         * Forward const iterator pointing to the first entry (if any)
         **/
        const_iterator begin() const { return _dirs.begin(); }
        /**
         * Forward const iterator pointing behind the last entry.
         **/
        const_iterator end() const { return _dirs.end(); }
        /**
         * Reverse const iterator pointing to the last entry (if any)
         **/
        const_reverse_iterator rbegin() const { return _dirs.rbegin(); }
        /**
         * Reverse const iterator pointing before the first entry.
         **/
        const_reverse_iterator rend()const { return _dirs.rend(); }

      public:

        friend std::ostream & operator<<( std::ostream & str, const PackageImplIf::DiskUsage & obj );

      };
#if 0
    
      /**
       * @short Holds Data about file and file type
       *  (directory, plain)
       **/
      class FileData {
      public:
        std::string name;
        std::string type;
        FileData();
        FileData(const std::string &name,
                 const std::string &type)
	: name(name), type(type)
	{}
      };
#endif

    public:
      /** \name Rpm Package Attributes. */
      //@{
      /** */
      virtual Date buildtime() const PURE_VIRTUAL;
      /** */
      virtual std::string buildhost() const PURE_VIRTUAL;
      /** */
      virtual Date installtime() const PURE_VIRTUAL;
      /** */
      virtual std::string distribution() const PURE_VIRTUAL;
      /** */
      virtual Vendor vendor() const PURE_VIRTUAL;
      /** */
      virtual Label license() const PURE_VIRTUAL;
      /** */
      virtual std::string packager() const PURE_VIRTUAL;
      /** */
      virtual PackageGroup group() const PURE_VIRTUAL;
      /** */
      virtual Changelog changelog() const PURE_VIRTUAL;
      /** Don't ship it as class Url, because it might be
       * in fact anything but a legal Url. */
      virtual std::string url() const PURE_VIRTUAL;
      /** */
      virtual std::string os() const PURE_VIRTUAL;
      /** */
      virtual Text prein() const PURE_VIRTUAL;
      /** */
      virtual Text postin() const PURE_VIRTUAL;
      /** */
      virtual Text preun() const PURE_VIRTUAL;
      /** */
      virtual Text postun() const PURE_VIRTUAL;
      /** */
      virtual ByteCount sourcesize() const PURE_VIRTUAL;
      /** */
      virtual ByteCount archivesize() const PURE_VIRTUAL;
      /** */
      virtual Text authors() const PURE_VIRTUAL;
      /** */
      virtual Text filenames() const PURE_VIRTUAL;
      //@}

      /** \name Additional Package Attributes.
       * \todo review what's actually needed here. Maybe worth grouping
       * all the package rertieval related stuff in a class. Easier to ship
       * and handle it.
      */
      //@{
      /** */
      virtual License licenseToConfirm() const PURE_VIRTUAL;
#if 0
      /** */
      virtual std::string sourceloc() const PURE_VIRTUAL;
      /** */
      virtual void du( PkgDu & dudata_r ) const PURE_VIRTUAL;
      /** */
      virtual std::string location() const PURE_VIRTUAL;
      /** */
      virtual unsigned int medianr() const PURE_VIRTUAL;
      /** */
      virtual PackageKeywords keywords() const PURE_VIRTUAL;
      /** */
      virtual std::string md5sum() const PURE_VIRTUAL;
      /** */
      virtual std::string externalUrl() const PURE_VIRTUAL;
      /** */
      virtual std::list<Edition> patchRpmBaseVersions() const PURE_VIRTUAL;
      /** */
      virtual ByteCount patchRpmSize() const PURE_VIRTUAL;
      /** */
      virtual bool forceInstall() const PURE_VIRTUAL;
      /** */
      virtual std::string patchRpmMD5() const PURE_VIRTUAL;
      /** */
      virtual bool isRemote() const PURE_VIRTUAL;
      /** */
      virtual PMError providePkgToInstall( Pathname& path_r ) const PURE_VIRTUAL;
      /** */
      virtual PMError provideSrcPkgToInstall( Pathname& path_r ) const PURE_VIRTUAL;
      /** */
      virtual constInstSrcPtr source() const PURE_VIRTUAL;
      /** */
      virtual std::list<PMPackageDelta> deltas() const PURE_VIRTUAL;
#endif
      //@}
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_PACKAGEIMPLIF_H
