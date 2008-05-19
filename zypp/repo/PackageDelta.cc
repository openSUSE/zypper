/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/PackageDelta.cc
 *
*/
#include <iostream>
extern "C"
{
#include <satsolver/repo.h>
}

#include "zypp/base/LogTools.h"

#include "zypp/repo/PackageDelta.h"


using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////


    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj )
    {
      str
      << "PatchRpm[" << obj.name() << "-" << obj.edition() << "." << obj.arch()
      << "](" << obj.location()
      << '|' << obj.buildtime()
      << '|';
      return dumpRangeLine( str, obj.baseversions().begin(), obj.baseversions().end() )
      << ')';
    }

    DeltaRpm::DeltaRpm(const Repository & repo, sat::detail::IdType extraid)
      : _repo(repo)
    {
      MIL << "creating deltarpm from repo " << repo.name() << ", id " << extraid << endl;
      ::Dataiterator di;
      ::dataiterator_init(&di, repo.get(), extraid, 0, 0, SEARCH_EXTRA | SEARCH_NO_STORAGE_SOLVABLE);

      string locdir;
      string locname;
      string locevr;
      string locsuffix;
      OnMediaLocation loc;
      BaseVersion base;
      string seqname;
      string seqevr;
      string seqnum;

      if (::dataiterator_step(&di))
      {
        do
        {
          switch (di.key->name)
          {
          case DELTA_PACKAGE_NAME:
          {
            setName(IdString(di.kv.id).asString());
            break;
          }
          case DELTA_PACKAGE_EVR:
          {
            setEdition(Edition(IdString(di.kv.id).asString()));
            break;
          }
          case DELTA_PACKAGE_ARCH:
          {
            setArch(Arch(IdString(di.kv.id).asString()));
            break;
          }
          case DELTA_LOCATION_DIR:
          {
            locdir = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_LOCATION_NAME:
          {
            locname = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_LOCATION_EVR:
          {
            locevr = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_LOCATION_SUFFIX:
          {
            locsuffix = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_DOWNLOADSIZE:
          {
            loc.setDownloadSize(di.kv.num);
            break;
          }
          case DELTA_CHECKSUM:
          {
            loc.setChecksum(CheckSum::sha1(di.kv.str));
            break;
          }
          case DELTA_BASE_EVR:
          {
            base.setEdition(Edition(IdString(di.kv.id).asString()));
            break;
          }
          case DELTA_SEQ_NAME:
          {
            seqname = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_SEQ_EVR:
          {
            seqevr = IdString(di.kv.id).asString();
            break;
          }
          case DELTA_SEQ_NUM:
          {
            seqnum = di.kv.str;
            break;
          }
          default:
            WAR << "ingoring unknown attribute: " << IdString(di.key->name) << endl;
          }
        } while (::dataiterator_step(&di));
      }
      else
        ERR << "the extra does not exist in the repo" << endl;

      //! \todo FIXME here + in sat tools
      loc.setLocation(locdir + "/" + locname + "-" + locevr + "." + locsuffix); 
      setLocation(loc);
      base.setSequenceinfo(seqname + "-" + seqevr + "-" + seqnum);
      setBaseversion(base);
    }

    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj )
    {
      return str
      << "DeltaRpm[" << obj.name() << "-" << obj.edition() << "." << obj.arch()
      << "](" << obj.location()
      //<< '|' << obj.buildtime()
      << '|' << obj.baseversion().edition()
      //<< ',' << obj.baseversion().buildtime()
      //<< ',' << obj.baseversion().checksum()
      << ',' << obj.baseversion().sequenceinfo()
      << ')';
    }

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
