/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmHeader.cc
 *
*/
#include "librpm.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"

#include "zypp/target/rpm/RpmHeader.h"
#include "zypp/CapFactory.h"
#include "zypp/Rel.h"
#include "zypp/Package.h"
#include "zypp/base/Exception.h"

using namespace std;

namespace zypp {
  namespace target {
    namespace rpm {

      ///////////////////////////////////////////////////////////////////
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::RpmHeader
      //        METHOD TYPE : Constructor
      //
      //        DESCRIPTION :
      //
      RpmHeader::RpmHeader( Header h_r )
          : BinHeader( h_r )
      {
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::RpmHeader
      //        METHOD TYPE : Constructor
      //
      RpmHeader::RpmHeader( BinHeader::Ptr & rhs )
          : BinHeader( rhs )
      {
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::~RpmHeader
      //        METHOD TYPE : Destructor
      //
      //        DESCRIPTION :
      //
      RpmHeader::~RpmHeader()
      {
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::readPackage
      //        METHOD TYPE : constRpmHeaderPtr
      //
      RpmHeader::constPtr RpmHeader::readPackage( const Pathname & path_r,
              				  VERIFICATION verification_r )
      {
        PathInfo file( path_r );
        if ( ! file.isFile() ) {
          ERR << "Not a file: " << file << endl;
          return (RpmHeader*)0;
        }
      
        FD_t fd = ::Fopen( file.asString().c_str(), "r.ufdio" );
        if ( fd == 0 || ::Ferror(fd) ) {
          ERR << "Can't open file for reading: " << file << " (" << ::Fstrerror(fd) << ")" << endl;
          if ( fd )
            ::Fclose( fd );
          return (RpmHeader*)0;
        }
      
        rpmts ts = ::rpmtsCreate();
        unsigned vsflag = RPMVSF_DEFAULT;
        if ( verification_r & NODIGEST )
          vsflag |= _RPMVSF_NODIGESTS;
        if ( verification_r & NOSIGNATURE )
          vsflag |= _RPMVSF_NOSIGNATURES;
        ::rpmtsSetVSFlags( ts, rpmVSFlags(vsflag) );
      
        Header nh = 0;
        int res = ::rpmReadPackageFile( ts, fd, path_r.asString().c_str(), &nh );
      
        ts = ::rpmtsFree(ts);
      
        ::Fclose( fd );
      
        if ( ! nh ) {
          WAR << "Error reading header from " << path_r << " error(" << res << ")" << endl;
          return (RpmHeader*)0;
        }
      
        RpmHeader::constPtr h( new RpmHeader( nh ) );
        headerFree( nh ); // clear the reference set in ReadPackageFile
      
        MIL << h << " from " << path_r << endl;
        return h;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::dumpOn
      //        METHOD TYPE : ostream &
      //
      //        DESCRIPTION :
      //
      ostream & RpmHeader::dumpOn( ostream & str ) const
      {
        return BinHeader::dumpOn( str ) << '{' << tag_name() << "-"
		<< (tag_epoch().empty()?"":(tag_epoch()+":"))
		<< tag_version()
		<< (tag_release().empty()?"":(string("-")+tag_release()))
		<< ( isSrc() ? ".src}" : "}");
      }
      
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::isSrc
      //        METHOD TYPE : bool
      //
      bool RpmHeader::isSrc() const
      {
        return has_tag( RPMTAG_SOURCEPACKAGE );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_name
      //        METHOD TYPE : string
      //
      //        DESCRIPTION :
      //
      string RpmHeader::tag_name() const
      {
        return string_val( RPMTAG_NAME );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_epoch
      //        METHOD TYPE : string
      //
      //        DESCRIPTION :
      //
      string RpmHeader::tag_epoch() const
      {
	return string_val ( RPMTAG_EPOCH );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_version
      //        METHOD TYPE : string
      //
      //        DESCRIPTION :
      //
      string RpmHeader::tag_version() const
      {
	return string_val ( RPMTAG_VERSION );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_release
      //        METHOD TYPE : string
      //
      //        DESCRIPTION :
      //
      string RpmHeader::tag_release() const
      {
	return string_val( RPMTAG_RELEASE );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_edition
      //        METHOD TYPE : Edition
      //
      //        DESCRIPTION :
      //
      Edition RpmHeader::tag_edition () const
      {
	try {
	  return Edition( tag_version(), tag_release(), tag_epoch());
	}
	catch (Exception & excpt_r) {
	  WAR << "Package " << tag_name() << "has an invalid edition";
	  ZYPP_CAUGHT (excpt_r);
	}
	return Edition();
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_arch
      //        METHOD TYPE : string
      //
      //        DESCRIPTION :
      //
      string RpmHeader::tag_arch() const
      {
        return string_val( RPMTAG_ARCH );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_installtime
      //        METHOD TYPE : Date
      //
      //        DESCRIPTION :
      //
      Date RpmHeader::tag_installtime() const
      {
        return int_val( RPMTAG_INSTALLTIME );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_buildtime
      //        METHOD TYPE : Date
      //
      //        DESCRIPTION :
      //
      Date RpmHeader::tag_buildtime() const
      {
        return int_val( RPMTAG_BUILDTIME );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::PkgRelList_val
      //        METHOD TYPE : unsigned
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::PkgRelList_val( tag tag_r, bool pre, set<string> * freq_r ) const
      {
        CapSet ret;
      
        int_32  kindFlags   = 0;
        int_32  kindVersion = 0;
      
        switch ( tag_r ) {
        case RPMTAG_REQUIRENAME:
          kindFlags   = RPMTAG_REQUIREFLAGS;
          kindVersion = RPMTAG_REQUIREVERSION;
          break;
        case RPMTAG_PROVIDENAME:
          kindFlags   = RPMTAG_PROVIDEFLAGS;
          kindVersion = RPMTAG_PROVIDEVERSION;
          break;
        case RPMTAG_OBSOLETENAME:
          kindFlags   = RPMTAG_OBSOLETEFLAGS;
          kindVersion = RPMTAG_OBSOLETEVERSION;
          break;
        case RPMTAG_CONFLICTNAME:
          kindFlags   = RPMTAG_CONFLICTFLAGS;
          kindVersion = RPMTAG_CONFLICTVERSION;
          break;
        case RPMTAG_ENHANCESNAME:
          kindFlags   = RPMTAG_ENHANCESFLAGS;
          kindVersion = RPMTAG_ENHANCESVERSION;
          break;
#warning NEEDS RPMTAG_SUPPLEMENTSNAME
#if 0
        case RPMTAG_SUPPLEMENTSNAME:
          kindFlags   = RPMTAG_SUPPLEMENTSFLAGS;
          kindVersion = RPMTAG_SUPPLEMENTSVERSION;
          break;
#endif
        default:
          INT << "Illegal RPMTAG_dependencyNAME " << tag_r << endl;
	  return ret;
          break;
        }
      
        stringList names;
        unsigned count = string_list( tag_r, names );
        if ( !count )
          return ret;
      
        intList  flags;
        int_list( kindFlags, flags );
      
        stringList versions;
        string_list( kindVersion, versions );
      
        for ( unsigned i = 0; i < count; ++i ) {
      
          string n( names[i] );
      
          Rel op = Rel::ANY;
          int_32 f  = flags[i];
          string v  = versions[i];
      
          if ( n[0] == '/' ) {
            if ( freq_r ) {
              freq_r->insert( n );
            }
          } else {
            if ( v.size() ) {
              switch ( f & RPMSENSE_SENSEMASK ) {
              case RPMSENSE_LESS:
                op = Rel::LT;
                break;
              case RPMSENSE_LESS|RPMSENSE_EQUAL:
                op = Rel::LE;
                break;
              case RPMSENSE_GREATER:
                op = Rel::GT;
                break;
              case RPMSENSE_GREATER|RPMSENSE_EQUAL:
                op = Rel::GE;
                break;
              case RPMSENSE_EQUAL:
                op = Rel::EQ;
                break;
              }
            }
          }
	  if ((pre && (f & RPMSENSE_PREREQ))
	    || ((! pre) && !(f & RPMSENSE_PREREQ)))
	  {
	    CapFactory _f;
	    try {
	      Capability cap = _f.parse(
	        ResTraits<Package>::kind,
	        n,
	        op,
	        Edition(v)
	      );
	      ret.insert(cap);
	    }
	    catch (Exception & excpt_r)
	    {
	      ZYPP_CAUGHT(excpt_r);
	      WAR << "Invalid capability: " << n << " " << op << " "
		<< v << endl;
	    }
	  }
        }
      
        return ret;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_provides
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_provides( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_PROVIDENAME, false, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_requires
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_requires( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_REQUIRENAME, false, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_requires
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_prerequires( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_REQUIRENAME, true, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_conflicts
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_conflicts( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_CONFLICTNAME, false, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_obsoletes
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_obsoletes( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_OBSOLETENAME, false, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_enhances
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_enhances( set<string> * freq_r ) const
      {
        return PkgRelList_val( RPMTAG_ENHANCESNAME, false, freq_r );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_supplements
      //        METHOD TYPE : CapSet
      //
      //        DESCRIPTION :
      //
      CapSet RpmHeader::tag_supplements( set<string> * freq_r ) const
      {
	return CapSet();
#warning NEEDS RPMTAG_SUPPLEMENTSNAME
#if 0
        return PkgRelList_val( RPMTAG_SUPPLEMENTSNAME, false, freq_r );
#endif
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_size
      //        METHOD TYPE : ByteCount
      //
      //        DESCRIPTION :
      //
      ByteCount RpmHeader::tag_size() const
      {
        return int_val( RPMTAG_SIZE );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_archivesize
      //        METHOD TYPE : ByteCount
      //
      //        DESCRIPTION :
      //
      ByteCount RpmHeader::tag_archivesize() const
      {
        return int_val( RPMTAG_ARCHIVESIZE );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_summary
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_summary() const
      {
        return string_val( RPMTAG_SUMMARY );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_description
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_description() const
      {
        return string_val( RPMTAG_DESCRIPTION );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_group
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_group() const
      {
        return string_val( RPMTAG_GROUP );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_vendor
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_vendor() const
      {
        return string_val( RPMTAG_VENDOR );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_distribution
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_distribution() const
      {
        return string_val( RPMTAG_DISTRIBUTION );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_license
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_license() const
      {
        return string_val( RPMTAG_LICENSE );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_buildhost
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_buildhost() const
      {
        return string_val( RPMTAG_BUILDHOST );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_packager
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_packager() const
      {
        return string_val( RPMTAG_PACKAGER );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_url
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_url() const
      {
        return string_val( RPMTAG_URL );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_os
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_os() const
      {
        return string_val( RPMTAG_OS );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_prein
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_prein() const
      {
        return string_val( RPMTAG_PREIN );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_postin
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_postin() const
      {
        return string_val( RPMTAG_POSTIN );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_preun
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_preun() const
      {
        return string_val( RPMTAG_PREUN );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_postun
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_postun() const
      {
        return string_val( RPMTAG_POSTUN );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_sourcerpm
      //        METHOD TYPE : std::string
      //
      //        DESCRIPTION :
      //
      std::string RpmHeader::tag_sourcerpm() const
      {
        return string_val( RPMTAG_SOURCERPM );
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_filenames
      //        METHOD TYPE : std::list<std::string>
      //
      //        DESCRIPTION :
      //
      std::list<std::string> RpmHeader::tag_filenames() const
      {
        std::list<std::string> ret;
      
        stringList basenames;
        if ( string_list( RPMTAG_BASENAMES, basenames ) ) {
          stringList dirnames;
          string_list( RPMTAG_DIRNAMES, dirnames );
          intList  dirindexes;
          int_list( RPMTAG_DIRINDEXES, dirindexes );
          for ( unsigned i = 0; i < basenames.size(); ++ i ) {
            ret.push_back( dirnames[dirindexes[i]] + basenames[i] );
          }
        }
      
        return ret;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_fileinfos
      //        METHOD TYPE : std::list<FileInfo>
      //
      //        DESCRIPTION :
      //
      std::list<FileInfo> RpmHeader::tag_fileinfos() const
      {
        std::list<FileInfo> ret;
      
        stringList basenames;
        if ( string_list( RPMTAG_BASENAMES, basenames ) ) {
          stringList dirnames;
          string_list( RPMTAG_DIRNAMES, dirnames );
          intList  dirindexes;
          int_list( RPMTAG_DIRINDEXES, dirindexes );
	  intList filesizes;
	  int_list( RPMTAG_FILESIZES, filesizes );
	  stringList md5sums;
	  string_list( RPMTAG_FILEMD5S, md5sums );
	  stringList usernames;
	  string_list( RPMTAG_FILEUSERNAME, usernames );
	  stringList groupnames;
	  string_list( RPMTAG_FILEGROUPNAME, groupnames );
	  intList uids;
	  int_list( RPMTAG_FILEUIDS, uids );
	  intList gids;
	  int_list( RPMTAG_FILEGIDS, gids );
	  intList filemodes;
	  int_list( RPMTAG_FILEMODES, filemodes );
	  intList filemtimes;
	  int_list( RPMTAG_FILEMTIMES, filemtimes );
	  intList fileflags;
	  int_list( RPMTAG_FILEFLAGS, fileflags );
	  stringList filelinks;
	  string_list( RPMTAG_FILELINKTOS, filelinks );

          for ( unsigned i = 0; i < basenames.size(); ++ i ) {
	    uid_t uid;
	    if (uids.empty()) {
	      uid = unameToUid( usernames[i].c_str(), &uid );
	    }
	    else {
	      uid =uids[i];
	    }

	    gid_t gid;
	    if (gids.empty()) {
	      gid = gnameToGid( groupnames[i].c_str(), &gid );
	    }
	    else {
	      gid = gids[i];
	    }

	    FileInfo info = {
	      dirnames[dirindexes[i]] + basenames[i],
	      filesizes[i],
	      md5sums[i],
	      uid,
	      gid,
	      filemodes[i],
	      filemtimes[i],
	      fileflags[i] & RPMFILE_GHOST,
	      filelinks[i]
	    };

            ret.push_back( info );
          }
        }
      
        return ret;
      }
      
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_changelog
      //        METHOD TYPE : Changelog
      //
      //        DESCRIPTION :
      //
      Changelog RpmHeader::tag_changelog() const
      {
        Changelog ret;
      
        intList times;
        if ( int_list( RPMTAG_CHANGELOGTIME, times ) ) {
          stringList names;
          string_list( RPMTAG_CHANGELOGNAME, names );
          stringList texts;
          string_list( RPMTAG_CHANGELOGTEXT, texts );
          for ( unsigned i = 0; i < times.size(); ++ i ) {
            ret.push_back( ChangelogEntry( times[i], names[i], texts[i] ) );
          }
        }
      
        return ret;
      }
     
      ///////////////////////////////////////////////////////////////////
      //
      //
      //        METHOD NAME : RpmHeader::tag_du
      //        METHOD TYPE : PkgDu &
      //
      //        DESCRIPTION :
      //
      DiskUsage & RpmHeader::tag_du( DiskUsage & dudata_r ) const
      {
        dudata_r.clear();
        stringList basenames;
        if ( string_list( RPMTAG_BASENAMES, basenames ) ) {
          stringList dirnames;
          string_list( RPMTAG_DIRNAMES, dirnames );
          intList dirindexes;
          int_list( RPMTAG_DIRINDEXES, dirindexes );
      
          intList filedevices;
          int_list( RPMTAG_FILEDEVICES, filedevices );
          intList fileinodes;
          int_list( RPMTAG_FILEINODES, fileinodes );
          intList filesizes;
          int_list( RPMTAG_FILESIZES, filesizes );
          intList filemodes;
          int_list( RPMTAG_FILEMODES, filemodes );
      
          ///////////////////////////////////////////////////////////////////
          // Create and collect Entries by index. devino_cache is used to
          // filter out hardliks ( different name but same device and inode ).
          ///////////////////////////////////////////////////////////////////
          filesystem::DevInoCache trace;
          vector<DiskUsage::Entry> entries;
          entries.resize( dirnames.size() );
          for ( unsigned i = 0; i < dirnames.size(); ++i ) {
            entries[i] = DiskUsage::Entry(dirnames[i]);
          }
      
          for ( unsigned i = 0; i < basenames.size(); ++ i ) {
            filesystem::StatMode mode( filemodes[i] );
            if ( mode.isFile() ) {
              if ( trace.insert( filedevices[i], fileinodes[i] ) ) {
                // Count full 1K blocks
                entries[dirindexes[i]]._size += ByteCount( filesizes[i] ).fillBlock();
                ++(entries[dirindexes[i]]._files);
              }
              // else: hardlink; already counted this device/inode
            }
          }
      
          ///////////////////////////////////////////////////////////////////
          // Crreate and collect by index Entries. DevInoTrace is used to
          // filter out hardliks ( different name but same device and inode ).
          ///////////////////////////////////////////////////////////////////
          for ( unsigned i = 0; i < entries.size(); ++i ) {
            if ( entries[i]._size ) {
              dudata_r.add( entries[i] );
            }
          }
        }
        return dudata_r;
      }

    } // namespace rpm
  } // namespace target
} // namespace zypp
