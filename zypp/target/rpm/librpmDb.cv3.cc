/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/librpmDb.cv3.cc
 *
 */
#if 0
#include "librpm.h"
extern "C"
{
#ifdef _RPM_5
typedef rpmuint32_t rpm_count_t;
#define HGEPtr_t void *
#define headerGetEntryMinMemory headerGetEntry
#define headerNVR(h,n,v,r) headerNEVRA(h,n,NULL,v,r,NULL)
#else
#ifdef _RPM_4_4
typedef int32_t rpm_count_t;
#define HGEPtr_t const void *
#endif
#endif
}

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/target/rpm/RpmCallbacks.h"
#include "zypp/ZYppCallbacks.h"

#define xmalloc malloc
#define xstrdup strdup

extern "C"
{
#include <string.h>

#define FA_MAGIC      0x02050920

  struct faFileHeader
  {
    unsigned int magic;
    unsigned int firstFree;
  };

  struct faHeader
  {
    unsigned int size;
    unsigned int freeNext; /* offset of the next free block, 0 if none */
    unsigned int freePrev;
    unsigned int isFree;

    /* note that the u16's appear last for alignment/space reasons */
  };
}

namespace zypp
{
namespace target
{
namespace rpm
{
static int fadFileSize;

static ssize_t Pread(FD_t fd, void * buf, size_t count, off_t offset)
{
  if (Fseek(fd, offset, SEEK_SET) < 0)
    return -1;
  return Fread(buf, sizeof(char), count, fd);
}

static FD_t fadOpen(const char * path)
{
  struct faFileHeader newHdr;
  FD_t fd;
  struct stat stb;

  fd = Fopen(path, "r.fdio");
  if (!fd || Ferror(fd))
    return NULL;

  if (fstat(Fileno(fd), &stb))
  {
    Fclose(fd);
    return NULL;
  }
  fadFileSize = stb.st_size;

  /* is this file brand new? */
  if (fadFileSize == 0)
  {
    Fclose(fd);
    return NULL;
  }
  if (Pread(fd, &newHdr, sizeof(newHdr), 0) != sizeof(newHdr))
  {
    Fclose(fd);
    return NULL;
  }
  if (newHdr.magic != FA_MAGIC)
  {
    Fclose(fd);
    return NULL;
  }
  /*@-refcounttrans@*/ return fd /*@=refcounttrans@*/ ;
}

static int fadNextOffset(FD_t fd, unsigned int lastOffset)
{
  struct faHeader header;
  int offset;

  offset = (lastOffset)
           ? (lastOffset - sizeof(header))
           : sizeof(struct faFileHeader);

  if (offset >= fadFileSize)
    return 0;

  if (Pread(fd, &header, sizeof(header), offset) != sizeof(header))
    return 0;

  if (!lastOffset && !header.isFree)
    return (offset + sizeof(header));

  do
  {
    offset += header.size;

    if (Pread(fd, &header, sizeof(header), offset) != sizeof(header))
      return 0;

    if (!header.isFree) break;
  }
  while (offset < fadFileSize && header.isFree);

  if (offset < fadFileSize)
  {
    /* Sanity check this to make sure we're not going in loops */
    offset += sizeof(header);

    if (offset < 0 || (unsigned)offset <= lastOffset) return -1;

    return offset;
  }
  else
    return 0;
}

static int fadFirstOffset(FD_t fd)
{
  return fadNextOffset(fd, 0);
}

/*@-boundsread@*/
static int dncmp(const void * a, const void * b)
/*@*/
{
  const char *const * first = (const char *const *)a;
  const char *const * second = (const char *const *)b;
  return strcmp(*first, *second);
}
/*@=boundsread@*/

/*@-bounds@*/
#ifndef _RPM_4_X
static void compressFilelist(Header h)
/*@*/
{
  char ** fileNames;
  const char ** dirNames;
  const char ** baseNames;
  int_32 * dirIndexes;
  rpmTagType fnt;
  rpm_count_t count;
  int xx;
  int dirIndex = -1;

  /*
   * This assumes the file list is already sorted, and begins with a
   * single '/'. That assumption isn't critical, but it makes things go
   * a bit faster.
   */

  if (headerIsEntry(h, RPMTAG_DIRNAMES))
  {
    xx = headerRemoveEntry(h, RPMTAG_OLDFILENAMES);
    return;		/* Already converted. */
  }

  HGEPtr_t hgePtr = NULL;
  if (!headerGetEntryMinMemory(h, RPMTAG_OLDFILENAMES, hTYP_t(&fnt), &hgePtr, &count))
    return;		/* no file list */
  fileNames = (char **)hgePtr;
  if (fileNames == NULL || count <= 0)
    return;

  dirNames = (const char **)alloca(sizeof(*dirNames) * count);	/* worst case */
  baseNames = (const char **)alloca(sizeof(*dirNames) * count);
  dirIndexes = (int_32 *)alloca(sizeof(*dirIndexes) * count);

  if (fileNames[0][0] != '/')
  {
    /* HACK. Source RPM, so just do things differently */
    dirIndex = 0;
    dirNames[dirIndex] = "";
    for (rpm_count_t i = 0; i < count; i++)
    {
      dirIndexes[i] = dirIndex;
      baseNames[i] = fileNames[i];
    }
    goto exit;
  }

  /*@-branchstate@*/
  for (rpm_count_t i = 0; i < count; i++)
  {
    const char ** needle;
    char savechar;
    char * baseName;
    int len;

    if (fileNames[i] == NULL)	/* XXX can't happen */
      continue;
    baseName = strrchr(fileNames[i], '/') + 1;
    len = baseName - fileNames[i];
    needle = dirNames;
    savechar = *baseName;
    *baseName = '\0';
    /*@-compdef@*/
    if (dirIndex < 0 ||
        (needle = (const char **)bsearch(&fileNames[i], dirNames, dirIndex + 1, sizeof(dirNames[0]), dncmp)) == NULL)
    {
      char *s = (char *)alloca(len + 1);
      memcpy(s, fileNames[i], len + 1);
      s[len] = '\0';
      dirIndexes[i] = ++dirIndex;
      dirNames[dirIndex] = s;
    }
    else
      dirIndexes[i] = needle - dirNames;
    /*@=compdef@*/

    *baseName = savechar;
    baseNames[i] = baseName;
  }
  /*@=branchstate@*/

exit:
  if (count > 0)
  {
    xx = headerAddEntry(h, RPMTAG_DIRINDEXES, RPM_INT32_TYPE, dirIndexes, count);
    xx = headerAddEntry(h, RPMTAG_BASENAMES, RPM_STRING_ARRAY_TYPE,
             baseNames, count);
    xx = headerAddEntry(h, RPMTAG_DIRNAMES, RPM_STRING_ARRAY_TYPE,
             dirNames, dirIndex + 1);
  }

  fileNames = (char**)headerFreeData(fileNames, fnt);

  xx = headerRemoveEntry(h, RPMTAG_OLDFILENAMES);
}
/*@=bounds@*/

/*
 * Up to rpm 3.0.4, packages implicitly provided their own name-version-release.
 * Retrofit an explicit "Provides: name = epoch:version-release".
 */
void providePackageNVR(Header h)
{
  const char *name, *version, *release;
  HGEPtr_t hgePtr = NULL;
  int_32 * epoch;
  const char *pEVR;
  char *p;
  int_32 pFlags = RPMSENSE_EQUAL;
  const char ** provides = NULL;
  const char ** providesEVR = NULL;
  rpmTagType pnt, pvt;
  int_32 * provideFlags = NULL;
  rpm_count_t providesCount;
  int xx;
  int bingo = 1;

  /* Generate provides for this package name-version-release. */
  xx = headerNVR(h, &name, &version, &release);
  if (!(name && version && release))
    return;
  pEVR = p = (char *)alloca(21 + strlen(version) + 1 + strlen(release) + 1);
  *p = '\0';
  if (headerGetEntryMinMemory(h, RPMTAG_EPOCH, NULL, &hgePtr, NULL))
  {
    epoch = (int_32 *)hgePtr;
    sprintf(p, "%d:", *epoch);
    while (*p != '\0')
      p++;
  }
  (void) stpcpy( stpcpy( stpcpy(p, version) , "-") , release);

  /*
   * Rpm prior to 3.0.3 does not have versioned provides.
   * If no provides at all are available, we can just add.
   */
  if (!headerGetEntryMinMemory(h, RPMTAG_PROVIDENAME, hTYP_t(&pnt), &hgePtr, &providesCount))
    goto exit;
  provides = (const char **)hgePtr;

  /*
   * Otherwise, fill in entries on legacy packages.
   */
  if (!headerGetEntryMinMemory(h, RPMTAG_PROVIDEVERSION, hTYP_t(&pvt), &hgePtr, NULL))
  {
    providesEVR = (const char **)hgePtr;
    for (rpm_count_t i = 0; i < providesCount; i++)
    {
      const char * vdummy = "";
      int_32 fdummy = RPMSENSE_ANY;
      xx = headerAddOrAppendEntry(h, RPMTAG_PROVIDEVERSION, RPM_STRING_ARRAY_TYPE,
                                  &vdummy, 1);
      xx = headerAddOrAppendEntry(h, RPMTAG_PROVIDEFLAGS, RPM_INT32_TYPE,
                                  &fdummy, 1);
    }
    goto exit;
  }

  xx = headerGetEntryMinMemory(h, RPMTAG_PROVIDEFLAGS, NULL, &hgePtr, NULL);
  provideFlags = (int_32 *)hgePtr;

  /*@-nullderef@*/    /* LCL: providesEVR is not NULL */
  if (provides && providesEVR && provideFlags)
    for (rpm_count_t i = 0; i < providesCount; i++)
    {
      if (!(provides[i] && providesEVR[i]))
        continue;
      if (!(provideFlags[i] == RPMSENSE_EQUAL &&
            !strcmp(name, provides[i]) && !strcmp(pEVR, providesEVR[i])))
        continue;
      bingo = 0;
      break;
    }
  /*@=nullderef@*/

exit:
  provides = (const char **)headerFreeData(provides, pnt);
  providesEVR = (const char **)headerFreeData(providesEVR, pvt);

  if (bingo)
  {
    xx = headerAddOrAppendEntry(h, RPMTAG_PROVIDENAME, RPM_STRING_ARRAY_TYPE,
                                &name, 1);
    xx = headerAddOrAppendEntry(h, RPMTAG_PROVIDEFLAGS, RPM_INT32_TYPE,
                                &pFlags, 1);
    xx = headerAddOrAppendEntry(h, RPMTAG_PROVIDEVERSION, RPM_STRING_ARRAY_TYPE,
                                &pEVR, 1);
  }
}
#else
static void compressFilelist(Header h)
{
    struct rpmtd_s fileNames;
    char ** dirNames;
    const char ** baseNames;
    uint32_t * dirIndexes;
    rpm_count_t count;
    int xx, i;
    int dirIndex = -1;

    /*
     * This assumes the file list is already sorted, and begins with a
     * single '/'. That assumption isn't critical, but it makes things go
     * a bit faster.
     */

    if (headerIsEntry(h, RPMTAG_DIRNAMES)) {
        xx = headerDel(h, RPMTAG_OLDFILENAMES);
        return;         /* Already converted. */
    }

    if (!headerGet(h, RPMTAG_OLDFILENAMES, &fileNames, HEADERGET_MINMEM))
        return;
    count = rpmtdCount(&fileNames);
    if (count < 1)
        return;

    dirNames = (char**)malloc(sizeof(*dirNames) * count);      /* worst case */
    baseNames = (const char**)malloc(sizeof(*dirNames) * count);
    dirIndexes = (uint32_t*)malloc(sizeof(*dirIndexes) * count);

    /* HACK. Source RPM, so just do things differently */
    {   const char *fn = rpmtdGetString(&fileNames);
        if (fn && *fn != '/') {
            dirIndex = 0;
            dirNames[dirIndex] = xstrdup("");
            while ((i = rpmtdNext(&fileNames)) >= 0) {
                dirIndexes[i] = dirIndex;
                baseNames[i] = rpmtdGetString(&fileNames);
            }
            goto exit;
        }
    }

    while ((i = rpmtdNext(&fileNames)) >= 0) {
        char ** needle;
        char savechar;
        char * baseName;
        size_t len;
        const char *filename = rpmtdGetString(&fileNames);

        if (filename == NULL)   /* XXX can't happen */
            continue;
        baseName = strrchr((char*)filename, '/') + 1;
        len = baseName - filename;
        needle = dirNames;
        savechar = *baseName;
        *baseName = '\0';
        if (dirIndex < 0 ||
            (needle = (char**)bsearch(&filename, dirNames, dirIndex + 1, sizeof(dirNames[0]), dncmp)) == NULL) {
            char *s = (char*)malloc(len + 1);
            rstrlcpy(s, filename, len + 1);
            dirIndexes[i] = ++dirIndex;
            dirNames[dirIndex] = s;
        } else
            dirIndexes[i] = needle - dirNames;

        *baseName = savechar;
        baseNames[i] = baseName;
    }

exit:
    if (count > 0) {
        headerPutUint32(h, RPMTAG_DIRINDEXES, dirIndexes, count);
        headerPutStringArray(h, RPMTAG_BASENAMES, baseNames, count);
        headerPutStringArray(h, RPMTAG_DIRNAMES,
                             (const char **) dirNames, dirIndex + 1);
    }

    rpmtdFreeData(&fileNames);
    for (i = 0; i <= dirIndex; i++) {
        free(dirNames[i]);
    }
    free(dirNames);
    free(baseNames);
    free(dirIndexes);

    xx = headerDel(h, RPMTAG_OLDFILENAMES);
}

/*
 * Up to rpm 3.0.4, packages implicitly provided their own name-version-release.
 * Retrofit an explicit "Provides: name = epoch:version-release.
 */
static void providePackageNVR(Header h)
{
    const char *name;
    char *pEVR;
    rpmsenseFlags pFlags = RPMSENSE_EQUAL;
    int bingo = 1;
    struct rpmtd_s pnames;
    rpmds hds, nvrds;

    /* Generate provides for this package name-version-release. */
    pEVR = headerGetEVR(h, &name);
    if (!(name && pEVR))
        return;

    /*
     * Rpm prior to 3.0.3 does not have versioned provides.
     * If no provides at all are available, we can just add.
     */
    if (!headerGet(h, RPMTAG_PROVIDENAME, &pnames, HEADERGET_MINMEM)) {
        goto exit;
    }

    /*
     * Otherwise, fill in entries on legacy packages.
     */
    if (!headerIsEntry(h, RPMTAG_PROVIDEVERSION)) {
        while (rpmtdNext(&pnames) >= 0) {
            uint32_t fdummy = RPMSENSE_ANY;

            headerPutString(h, RPMTAG_PROVIDEVERSION, "");
            headerPutUint32(h, RPMTAG_PROVIDEFLAGS, &fdummy, 1);
        }
        goto exit;
    }

    /* see if we already have this provide */
    hds = rpmdsNew(h, RPMTAG_PROVIDENAME, 0);
    nvrds = rpmdsSingle(RPMTAG_PROVIDENAME, name, pEVR, pFlags);
    if (rpmdsFind(hds, nvrds) >= 0) {
        bingo = 0;
    }
    rpmdsFree(hds);
    rpmdsFree(nvrds);

exit:
    if (bingo) {
        const char *evr = pEVR;
	uint32_t fdummy = pFlags;
        headerPutString(h, RPMTAG_PROVIDENAME, name);
        headerPutString(h, RPMTAG_PROVIDEVERSION, evr);
        headerPutUint32(h, RPMTAG_PROVIDEFLAGS, &fdummy, 1);
    }
    rpmtdFreeData(&pnames);
    free(pEVR);
}

#endif

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

using namespace std;

#undef Y2LOG
#define Y2LOG "librpmDb"

/******************************************************************
**
**
**	FUNCTION NAME : internal_convertV3toV4
**	FUNCTION TYPE : int
*/
void internal_convertV3toV4( const Pathname & v3db_r, const librpmDb::constPtr & v4db_r,
                             callback::SendReport<ConvertDBReport> & report )
{
//  Timecount _t( "convert V3 to V4" );
  MIL << "Convert rpm3 database to rpm4" << endl;

  // Check arguments
  FD_t fd = fadOpen( v3db_r.asString().c_str() );
  if ( fd == 0 )
  {
    Fclose( fd );
    ZYPP_THROW(RpmDbOpenException(Pathname("/"), v3db_r));
  }

  if ( ! v4db_r )
  {
    Fclose( fd );
    INT << "NULL rpmV4 database passed as argument!" << endl;
    ZYPP_THROW(RpmNullDatabaseException());
  }

  shared_ptr<RpmException> err = v4db_r->error();
  if ( err )
  {
    Fclose( fd );
    INT << "Can't access rpmV4 database " << v4db_r << endl;
    ZYPP_THROW(*err);
  }

  // open rpmV4 database for writing. v4db_r is ok so librpm should
  // be properly initialized.
  rpmdb db = 0;
  string rootstr( v4db_r->root().asString() );
  const char * root = ( rootstr == "/" ? NULL : rootstr.c_str() );

  int res = ::rpmdbOpen( root, &db, O_RDWR, 0644 );
  if ( res || ! db )
  {
    if ( db )
    {
      ::rpmdbClose( db );
    }
    Fclose( fd );
    ZYPP_THROW(RpmDbOpenException(root, v4db_r->dbPath()));
  }

  // Check ammount of packages to process.
  int max = 0;
  for ( int offset = fadFirstOffset(fd); offset; offset = fadNextOffset(fd, offset) )
  {
    ++max;
  }
  MIL << "Packages in rpmV3 database " << v3db_r << ": " << max << endl;

  unsigned failed      = 0;
  unsigned ignored     = 0;
  unsigned alreadyInV4 = 0;
  report->progress( (100 * (failed + ignored + alreadyInV4) / max), v3db_r );

  if ( !max )
  {
    Fclose( fd );
    ::rpmdbClose( db );
    return;
  }

  // Start conversion.
#warning Add CBSuggest handling if needed, also on lines below
//  CBSuggest proceed;
  bool proceed = true;
  for ( int offset = fadFirstOffset(fd); offset && proceed /*!= CBSuggest::CANCEL*/;
        offset = fadNextOffset(fd, offset),
        report->progress( (100 * (failed + ignored + alreadyInV4) / max), v3db_r ) )
  {

    // have to use lseek instead of Fseek because headerRead
    // uses low level IO
    if ( lseek( Fileno( fd ), (off_t)offset, SEEK_SET ) == -1 )
    {
      ostream * reportAs = &(ERR);
      /*      proceed = report->dbReadError( offset );
            if ( proceed == CBSuggest::SKIP ) {
      	// ignore this error
      	++ignored;
      	reportAs = &(WAR << "IGNORED: ");
            } else {*/
      // PROCEED will fail after conversion; CANCEL immediately stop loop
      ++failed;
//      }
      (*reportAs) << "rpmV3 database entry: Can't seek to offset " << offset << " (errno " << errno << ")" << endl;
      continue;
    }
    Header h = headerRead(fd, HEADER_MAGIC_NO);
    if ( ! h )
    {
      ostream * reportAs = &(ERR);
      /*      proceed = report->dbReadError( offset );
            if ( proceed == CBSuggest::SKIP ) {
      	// ignore this error
      	++ignored;
      	reportAs = &(WAR << "IGNORED: ");
            } else {*/
      // PROCEED will fail after conversion; CANCEL immediately stop loop
      ++failed;
//      }
      (*reportAs) << "rpmV3 database entry: No header at offset " << offset << endl;
      continue;
    }
    compressFilelist(h);
    providePackageNVR(h);
    const char *name = 0;
    const char *version = 0;
    const char *release = 0;
    headerNVR(h, &name, &version, &release);
    string nrv( string(name) + "-" +  version + "-" + release );
    rpmdbMatchIterator mi = rpmdbInitIterator(db, RPMTAG_NAME, name, 0);
    rpmdbSetIteratorRE(mi, RPMTAG_VERSION, RPMMIRE_DEFAULT, version);
    rpmdbSetIteratorRE(mi, RPMTAG_RELEASE, RPMMIRE_DEFAULT, release);
    if (rpmdbNextIterator(mi))
    {
//      report.dbInV4( nrv );
      WAR << "SKIP: rpmV3 database entry: " << nrv << " is already in rpmV4 database" << endl;
      rpmdbFreeIterator(mi);
      headerFree(h);
      ++alreadyInV4;
      continue;
    }
    rpmdbFreeIterator(mi);
#ifdef _RPM_5
    if (rpmdbAdd(db, -1, h, 0))
#else
    if (rpmdbAdd(db, -1, h, 0, 0))
#endif
    {
//      report.dbWriteError( nrv );
      proceed = false;//CBSuggest::CANCEL; // immediately stop loop
      ++failed;
      ERR << "rpmV4 database error: could not add " << nrv << " to rpmV4 database" << endl;
      headerFree(h);
      continue;
    }
    headerFree(h);
  }

  Fclose(fd);
  ::rpmdbClose(db);

  if ( failed )
  {
    ERR << "Convert rpm3 database to rpm4: Aborted after "
    << alreadyInV4 << " package(s) and " << (failed+ignored) << " error(s)."
    << endl;
    ZYPP_THROW(RpmDbConvertException());
  }
  else
  {
    MIL << "Convert rpm3 database to rpm4: " << max << " package(s) processed";
    if ( alreadyInV4 )
    {
      MIL << "; " << alreadyInV4 << " already present in rpmV4 database";
    }
    if ( ignored )
    {
      MIL << "; IGNORED: " << ignored << " unconverted due to error";
    }
    MIL << endl;
  }
}
#endif

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/target/rpm/librpmDb.h"
#include "zypp/target/rpm/RpmCallbacks.h"
#include "zypp/ZYppCallbacks.h"

using namespace std;

#undef Y2LOG
#define Y2LOG "librpmDb"

namespace zypp
{
namespace target
{
namespace rpm
{
/******************************************************************
*
*
*	FUNCTION NAME : convertV3toV4
*
* \throws RpmException
*
*/
void convertV3toV4( const Pathname & v3db_r, const librpmDb::constPtr & v4db_r )
{
  // report
  callback::SendReport<ConvertDBReport> report;
  report->start(v3db_r);
  try
  {
    // Does no longer work with rpm 4.9.
    // internal_convertV3toV4( v3db_r, v4db_r, report );
    INT << "Unsupported: Convert rpm3 database to rpm4" << endl;
    ZYPP_THROW(RpmDbOpenException(Pathname("/"), v3db_r));
  }
  catch (RpmException & excpt_r)
  {
    report->finish(v3db_r, ConvertDBReport::FAILED,excpt_r.asUserString());
    ZYPP_RETHROW(excpt_r);
  }
  report->finish(v3db_r, ConvertDBReport::NO_ERROR, "");
}

} // namespace rpm
} // namespace target
} // namespace zypp
