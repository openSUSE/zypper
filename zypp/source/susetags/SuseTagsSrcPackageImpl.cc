/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsSrcPackageImpl.cc
 *
*/
#include "zypp/source/susetags/SuseTagsSrcPackageImpl.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SrcPackageImpl::SrcPackageImpl
      //	METHOD TYPE : Ctor
      //
      SuseTagsSrcPackageImpl::SuseTagsSrcPackageImpl(Source_Ref source_r)
      : _source( source_r )
      , _media_number( 1 )
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SrcPackageImpl::~SrcPackageImpl
      //	METHOD TYPE : Dtor
      //
      SuseTagsSrcPackageImpl::~SuseTagsSrcPackageImpl()
      {}


      Pathname SuseTagsSrcPackageImpl::location() const
      { return _location; }

      ByteCount SuseTagsSrcPackageImpl::archivesize() const
      { return _archivesize; }

      DiskUsage SuseTagsSrcPackageImpl::diskusage() const
      { return _diskusage; }

      Source_Ref SuseTagsSrcPackageImpl::source() const
      { return _source; }

      unsigned SuseTagsSrcPackageImpl::sourceMediaNr() const
      { return _media_number; }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
