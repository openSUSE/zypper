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
#ifdef _RPM_4_4
#include <rpm/ugid.h>
#else
////////////////////////////////////////////////////////////////////
// unameToUid and gnameToGid are shamelessly stolen from rpm-4.4.
// (rpmio/ugid.c) Those functions were dropped in RPM_4_7
extern "C"
{
#include <pwd.h>
#include <grp.h>
}
/* unameToUid(), uidTouname() and the group variants are really poorly
   implemented. They really ought to use hash tables. I just made the
   guess that most files would be owned by root or the same person/group
   who owned the last file. Those two values are cached, everything else
   is looked up via getpw() and getgr() functions.  If this performs
   too poorly I'll have to implement it properly :-( */

int unameToUid(const char * thisUname, uid_t * uid)
{
/*@only@*/ static char * lastUname = NULL;
    static size_t lastUnameLen = 0;
    static size_t lastUnameAlloced;
    static uid_t lastUid;
    struct passwd * pwent;
    size_t thisUnameLen;

    if (!thisUname) {
	lastUnameLen = 0;
	return -1;
    } else if (strcmp(thisUname, "root") == 0) {
/*@-boundswrite@*/
	*uid = 0;
/*@=boundswrite@*/
	return 0;
    }

    thisUnameLen = strlen(thisUname);
    if (lastUname == NULL || thisUnameLen != lastUnameLen ||
	strcmp(thisUname, lastUname) != 0)
    {
	if (lastUnameAlloced < thisUnameLen + 1) {
	    lastUnameAlloced = thisUnameLen + 10;
	    lastUname = (char *)realloc(lastUname, lastUnameAlloced);	/* XXX memory leak */
	}
/*@-boundswrite@*/
	strcpy(lastUname, thisUname);
/*@=boundswrite@*/

	pwent = getpwnam(thisUname);
	if (pwent == NULL) {
	    /*@-internalglobs@*/ /* FIX: shrug */
	    endpwent();
	    /*@=internalglobs@*/
	    pwent = getpwnam(thisUname);
	    if (pwent == NULL) return -1;
	}

	lastUid = pwent->pw_uid;
    }

/*@-boundswrite@*/
    *uid = lastUid;
/*@=boundswrite@*/

    return 0;
}

int gnameToGid(const char * thisGname, gid_t * gid)
{
/*@only@*/ static char * lastGname = NULL;
    static size_t lastGnameLen = 0;
    static size_t lastGnameAlloced;
    static gid_t lastGid;
    size_t thisGnameLen;
    struct group * grent;

    if (thisGname == NULL) {
	lastGnameLen = 0;
	return -1;
    } else if (strcmp(thisGname, "root") == 0) {
/*@-boundswrite@*/
	*gid = 0;
/*@=boundswrite@*/
	return 0;
    }

    thisGnameLen = strlen(thisGname);
    if (lastGname == NULL || thisGnameLen != lastGnameLen ||
	strcmp(thisGname, lastGname) != 0)
    {
	if (lastGnameAlloced < thisGnameLen + 1) {
	    lastGnameAlloced = thisGnameLen + 10;
	    lastGname = (char *)realloc(lastGname, lastGnameAlloced);	/* XXX memory leak */
	}
/*@-boundswrite@*/
	strcpy(lastGname, thisGname);
/*@=boundswrite@*/

	grent = getgrnam(thisGname);
	if (grent == NULL) {
	    /*@-internalglobs@*/ /* FIX: shrug */
	    endgrent();
	    /*@=internalglobs@*/
	    grent = getgrnam(thisGname);
	    if (grent == NULL) {
		/* XXX The filesystem package needs group/lock w/o getgrnam. */
		if (strcmp(thisGname, "lock") == 0) {
/*@-boundswrite@*/
		    *gid = lastGid = 54;
/*@=boundswrite@*/
		    return 0;
		} else
		if (strcmp(thisGname, "mail") == 0) {
/*@-boundswrite@*/
		    *gid = lastGid = 12;
/*@=boundswrite@*/
		    return 0;
		} else
		return -1;
	    }
	}
	lastGid = grent->gr_gid;
    }

/*@-boundswrite@*/
    *gid = lastGid;
/*@=boundswrite@*/

    return 0;
}
////////////////////////////////////////////////////////////////////
#endif

#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/target/rpm/RpmHeader.h"
#include "zypp/Package.h"
#include "zypp/PathInfo.h"

using std::endl;

namespace zypp
{
namespace target
{
namespace rpm
{

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
{}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::RpmHeader
//        METHOD TYPE : Constructor
//
RpmHeader::RpmHeader( BinHeader::Ptr & rhs )
    : BinHeader( rhs )
{}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::~RpmHeader
//        METHOD TYPE : Destructor
//
//        DESCRIPTION :
//
RpmHeader::~RpmHeader()
{}

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
  if ( ! file.isFile() )
  {
    ERR << "Not a file: " << file << endl;
    return (RpmHeader*)0;
  }

  FD_t fd = ::Fopen( file.asString().c_str(), "r.ufdio" );
  if ( fd == 0 || ::Ferror(fd) )
  {
    ERR << "Can't open file for reading: " << file << " (" << ::Fstrerror(fd) << ")" << endl;
    if ( fd )
      ::Fclose( fd );
    return (RpmHeader*)0;
  }

  librpmDb::globalInit();
  rpmts ts = ::rpmtsCreate();
  unsigned vsflag = RPMVSF_DEFAULT;
  if ( verification_r & NODIGEST )
    vsflag |= _RPMVSF_NODIGESTS;
  if ( verification_r & NOSIGNATURE )
    vsflag |= _RPMVSF_NOSIGNATURES;
  ::rpmtsSetVSFlags( ts, rpmVSFlags(vsflag) );

  Header nh = 0;
  int res = ::rpmReadPackageFile( ts, fd, path_r.asString().c_str(), &nh );

  ts = rpmtsFree(ts);

  ::Fclose( fd );

  if ( ! nh )
  {
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
//        METHOD TYPE : std::ostream &
//
//        DESCRIPTION :
//
std::ostream & RpmHeader::dumpOn( std::ostream & str ) const
{
  BinHeader::dumpOn( str ) << '{' << tag_name() << "-";
  if ( tag_epoch() != 0 )
    str << tag_epoch() << ":";
  str << tag_version()
      << (tag_release().empty()?"":(std::string("-")+tag_release()))
      << ( isSrc() ? ".src}" : "}");
  return str;
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

bool RpmHeader::isNosrc() const
{
  return has_tag( RPMTAG_SOURCEPACKAGE ) && ( has_tag( RPMTAG_NOSOURCE ) || has_tag( RPMTAG_NOPATCH ) );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_name
//        METHOD TYPE : std::string
//
//        DESCRIPTION :
//
std::string RpmHeader::tag_name() const
{
  return string_val( RPMTAG_NAME );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_epoch
//        METHOD TYPE : Edition::epoch_t
//
//        DESCRIPTION :
//
Edition::epoch_t RpmHeader::tag_epoch() const
{
  return int_val ( RPMTAG_EPOCH );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_version
//        METHOD TYPE : std::string
//
//        DESCRIPTION :
//
std::string RpmHeader::tag_version() const
{
  return string_val ( RPMTAG_VERSION );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_release
//        METHOD TYPE : std::string
//
//        DESCRIPTION :
//
std::string RpmHeader::tag_release() const
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
  return Edition( tag_version(), tag_release(), tag_epoch() );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_arch
//        METHOD TYPE : Arch
//
//        DESCRIPTION :
//
Arch RpmHeader::tag_arch() const
{
  return Arch( string_val( RPMTAG_ARCH ) );
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
#warning CHECK IF FILE REQUIRES HANDLING IS OBSOLETE
///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::PkgRelList_val
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::PkgRelList_val( tag tag_r, bool pre, std::set<std::string> * freq_r ) const
  {
    CapabilitySet ret;

    rpmTag  kindFlags   = rpmTag(0);
    rpmTag  kindVersion = rpmTag(0);

    switch ( tag_r )
    {
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
#ifdef RPMTAG_OLDSUGGESTS
    case RPMTAG_OLDENHANCESNAME:
      kindFlags   = RPMTAG_OLDENHANCESFLAGS;
      kindVersion = RPMTAG_OLDENHANCESVERSION;
      break;
    case RPMTAG_OLDSUGGESTSNAME:
      kindFlags   = RPMTAG_OLDSUGGESTSFLAGS;
      kindVersion = RPMTAG_OLDSUGGESTSVERSION;
      break;
    case RPMTAG_RECOMMENDNAME:
      kindFlags   = RPMTAG_RECOMMENDFLAGS;
      kindVersion = RPMTAG_RECOMMENDVERSION;
      break;
    case RPMTAG_SUPPLEMENTNAME:
      kindFlags   = RPMTAG_SUPPLEMENTFLAGS;
      kindVersion = RPMTAG_SUPPLEMENTVERSION;
      break;
    case RPMTAG_SUGGESTNAME:
      kindFlags   = RPMTAG_SUGGESTFLAGS;
      kindVersion = RPMTAG_SUGGESTVERSION;
      break;
    case RPMTAG_ENHANCENAME:
      kindFlags   = RPMTAG_ENHANCEFLAGS;
      kindVersion = RPMTAG_ENHANCEVERSION;
      break;
#else
    case RPMTAG_ENHANCESNAME:
      kindFlags   = RPMTAG_ENHANCESFLAGS;
      kindVersion = RPMTAG_ENHANCESVERSION;
      break;
    case RPMTAG_SUGGESTSNAME:
      kindFlags   = RPMTAG_SUGGESTSFLAGS;
      kindVersion = RPMTAG_SUGGESTSVERSION;
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

    for ( unsigned i = 0; i < count; ++i )
    {

      std::string n( names[i] );

      Rel op = Rel::ANY;
      int32_t f = flags[i];
      std::string v = versions[i];

      if ( n[0] == '/' )
      {
        if ( freq_r )
        {
          freq_r->insert( n );
        }
      }
      else
      {
        if ( v.size() )
        {
          switch ( f & RPMSENSE_SENSEMASK )
          {
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
        try
        {
          ret.insert( Capability( n, op, Edition(v) ) );
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
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_provides( std::set<std::string> * freq_r ) const
  {
    return PkgRelList_val( RPMTAG_PROVIDENAME, false, freq_r );
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_requires
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_requires( std::set<std::string> * freq_r ) const
  {
    return PkgRelList_val( RPMTAG_REQUIRENAME, false, freq_r );
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_requires
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_prerequires( std::set<std::string> * freq_r ) const
  {
    return PkgRelList_val( RPMTAG_REQUIRENAME, true, freq_r );
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_conflicts
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_conflicts( std::set<std::string> * freq_r ) const
  {
    return PkgRelList_val( RPMTAG_CONFLICTNAME, false, freq_r );
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_obsoletes
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_obsoletes( std::set<std::string> * freq_r ) const
  {
    return PkgRelList_val( RPMTAG_OBSOLETENAME, false, freq_r );
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_enhances
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_enhances( std::set<std::string> * freq_r ) const
  {
#ifdef RPMTAG_OLDSUGGESTS
    return PkgRelList_val( RPMTAG_ENHANCENAME, false, freq_r );
#else
    return PkgRelList_val( RPMTAG_ENHANCESNAME, false, freq_r );
#endif
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_suggests
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_suggests( std::set<std::string> * freq_r ) const
  {
#ifdef RPMTAG_OLDSUGGESTS
    return PkgRelList_val( RPMTAG_SUGGESTNAME, false, freq_r );
#else
    return PkgRelList_val( RPMTAG_SUGGESTSNAME, false, freq_r );
#endif
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_supplements
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_supplements( std::set<std::string> * freq_r ) const
  {
#ifdef RPMTAG_OLDSUGGESTS
    return PkgRelList_val( RPMTAG_SUPPLEMENTNAME, false, freq_r );
#else
    return CapabilitySet();
#endif
  }

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : RpmHeader::tag_recommends
//        METHOD TYPE : CapabilitySet
//
//        DESCRIPTION :
//
CapabilitySet RpmHeader::tag_recommends( std::set<std::string> * freq_r ) const
  {
#ifdef RPMTAG_OLDSUGGESTS
    return PkgRelList_val( RPMTAG_RECOMMENDNAME, false, freq_r );
#else
    return CapabilitySet();
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

std::string RpmHeader::tag_prein() const
{ return string_val( RPMTAG_PREIN ); }

std::string RpmHeader::tag_preinprog() const
{ return string_val( RPMTAG_PREINPROG ); }

std::string RpmHeader::tag_postin() const
{ return string_val( RPMTAG_POSTIN ); }

std::string RpmHeader::tag_postinprog() const
{ return string_val( RPMTAG_POSTINPROG ); }

std::string RpmHeader::tag_preun() const
{ return string_val( RPMTAG_PREUN ); }

std::string RpmHeader::tag_preunprog() const
{ return string_val( RPMTAG_PREUNPROG ); }

std::string RpmHeader::tag_postun() const
{ return string_val( RPMTAG_POSTUN ); }

std::string RpmHeader::tag_postunprog() const
{ return string_val( RPMTAG_POSTUNPROG ); }

std::string RpmHeader::tag_pretrans() const
{ return string_val( RPMTAG_PRETRANS ); }

std::string RpmHeader::tag_pretransprog() const
{ return string_val( RPMTAG_PRETRANSPROG ); }

std::string RpmHeader::tag_posttrans() const
{ return string_val( RPMTAG_POSTTRANS ); }

std::string RpmHeader::tag_posttransprog() const
{ return string_val( RPMTAG_POSTTRANSPROG ); }

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
  if ( string_list( RPMTAG_BASENAMES, basenames ) )
  {
    stringList dirnames;
    string_list( RPMTAG_DIRNAMES, dirnames );
    intList  dirindexes;
    int_list( RPMTAG_DIRINDEXES, dirindexes );
    for ( unsigned i = 0; i < basenames.size(); ++ i )
    {
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
  if ( string_list( RPMTAG_BASENAMES, basenames ) )
  {
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

    for ( unsigned i = 0; i < basenames.size(); ++ i )
    {
      uid_t uid;
      if (uids.empty())
      {
        uid = unameToUid( usernames[i].c_str(), &uid );
      }
      else
      {
        uid =uids[i];
      }

      gid_t gid;
      if (gids.empty())
      {
        gid = gnameToGid( groupnames[i].c_str(), &gid );
      }
      else
      {
        gid = gids[i];
      }

      FileInfo info = {
                        dirnames[dirindexes[i]] + basenames[i],
                        filesizes[i],
                        md5sums[i],
                        uid,
                        gid,
                        mode_t(filemodes[i]),
                        filemtimes[i],
                        bool(fileflags[i] & RPMFILE_GHOST),
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
  if ( int_list( RPMTAG_CHANGELOGTIME, times ) )
  {
    stringList names;
    string_list( RPMTAG_CHANGELOGNAME, names );
    stringList texts;
    string_list( RPMTAG_CHANGELOGTEXT, texts );
    for ( unsigned i = 0; i < times.size(); ++ i )
    {
      ret.push_back( ChangelogEntry( times[i], names[i], texts[i] ) );
    }
  }

  return ret;
}

} // namespace rpm
} // namespace target
} // namespace zypp
