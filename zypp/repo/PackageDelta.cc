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
#include <solv/knownid.h>
}

#include "zypp/base/LogTools.h"

#include "zypp/repo/PackageDelta.h"
#include "zypp/sat/Pool.h"


using std::endl;
using std::string;


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////

    DeltaRpm::DeltaRpm( sat::LookupAttr::iterator deltaInfo_r )
    {
      if ( deltaInfo_r.inSolvAttr() != sat::SolvAttr::repositoryDeltaInfo )
      {
        INT << "Illegal non-repositoryDeltaInfo iterator: " << deltaInfo_r << endl;
        return;
      }
      _repo = deltaInfo_r.inRepo();

      IdString locdir;
      IdString locname;
      IdString locevr;
      IdString locsuffix;

      IdString    seqname;
      IdString    seqevr;
      std::string seqnum;

      for_( it, deltaInfo_r.subBegin(), deltaInfo_r.subEnd() )
      {
        switch ( it.inSolvAttr().id() )
        {
          case DELTA_PACKAGE_NAME:
            _name = it.asString();
            break;

          case DELTA_PACKAGE_EVR:
            _edition = Edition( it.idStr() );
            break;

          case DELTA_PACKAGE_ARCH:
            _arch = Arch( it.idStr() );
            break;

          case DELTA_LOCATION_DIR:
            locdir = it.idStr();
            break;

          case DELTA_LOCATION_NAME:
            locname = it.idStr();
            break;

          case DELTA_LOCATION_EVR:
            locevr = it.idStr();
            break;

          case DELTA_LOCATION_SUFFIX:
            locsuffix = it.idStr();
            break;

          case DELTA_DOWNLOADSIZE:
            _location.setDownloadSize( ByteCount( it.asUnsignedLL() ) );
            break;

          case DELTA_CHECKSUM:
            _location.setChecksum( it.asCheckSum() );
            break;

          case DELTA_BASE_EVR:
            _baseversion.setEdition( Edition( it.idStr() ) );
            break;

          case DELTA_SEQ_NAME:
            seqname = it.idStr();
            break;

          case DELTA_SEQ_EVR:
            seqevr = it.idStr();
            break;

          case DELTA_SEQ_NUM:
            seqnum = it.asString();
            break;

          default:
            WAR << "Igore unknown attribute: " << it << endl;
        }
      }

      _location.setLocation( str::form( "%s/%s-%s.%s",
                                        locdir.c_str(),
                                        locname.c_str(),
                                        locevr.c_str(),
                                        locsuffix.c_str() ) );

      _baseversion.setSequenceinfo( str::form( "%s-%s-%s",
                                               seqname.c_str(),
                                               seqevr.c_str(),
                                               seqnum.c_str() ) );
    }

    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj )
    {
      return str
      << "DeltaRpm[" << obj.name() << "-" << obj.edition() << "." << obj.arch()
      << "](" << obj.location()
      << '|' << obj.baseversion().edition()
      << ',' << obj.baseversion().sequenceinfo()
      << ')';
    }

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
