/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/PackageDelta.h
 *
*/
#ifndef ZYPP_SOURCE_PACKAGEDELTA_H
#define ZYPP_SOURCE_PACKAGEDELTA_H

#include <iosfwd>
#include <list>

#include "zypp/source/OnMediaLocation.h"
#include "zypp/Edition.h"
#include "zypp/Date.h"
//#include "zypp/Arch.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////

    class PatchRpm
    {
    public:
      typedef Edition                BaseVersion;
      typedef std::list<BaseVersion> BaseVersions;

    public:
      PatchRpm()
      {}

    public:
      const source::OnMediaLocation & location()     const { return _location; }
      const BaseVersions &            baseversions() const { return _baseversions; }
      const Date &                    buildtime()    const { return _buildtime;}

    public:
      PatchRpm & location( const source::OnMediaLocation & val_r ) { _location = val_r; return *this; }
      PatchRpm & baseversions( const BaseVersions & val_r )        { _baseversions = val_r; return *this; }
      PatchRpm & baseversion( const BaseVersion & val_r )          { _baseversions.push_back( val_r ); return *this; }
      PatchRpm & buildtime( const Date & val_r )                   { _buildtime = val_r; return *this; }

    private:
      source::OnMediaLocation _location;
      BaseVersions            _baseversions;
      Date                    _buildtime;
    };

    /** \relates PatchRpm Stream output. */
    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj );

    ///////////////////////////////////////////////////////////////////

    class DeltaRpm
    {
    public:
      class BaseVersion
      {
      public:
        BaseVersion()
        {}

      public:
        const Edition &     edition()      const { return _edition; }
        const Date &        buildtime()    const { return _buildtime; }
        const CheckSum &    checksum()     const { return _checksum; }
        const std::string & sequenceinfo() const { return _sequenceinfo; }

      public:
        BaseVersion & edition( const Edition & val_r )          { _edition = val_r; return *this; }
        BaseVersion & buildtime( const Date & val_r )           { _buildtime = val_r; return *this; }
        BaseVersion & checksum( const CheckSum & val_r )        { _checksum = val_r; return *this; }
        BaseVersion & sequenceinfo( const std::string & val_r ) { _sequenceinfo = val_r; return *this; }

      private:
        Edition     _edition;
        Date        _buildtime;
        CheckSum    _checksum;
        std::string _sequenceinfo;
      };

      typedef std::list<BaseVersion> BaseVersions;

    public:
      DeltaRpm()
      {}

    public:
      const source::OnMediaLocation & location()     const { return _location; }
      const BaseVersion &             baseversion()  const { return _baseversion; }
      const Date &                    buildtime()    const { return _buildtime;}

    public:
      DeltaRpm & location( const source::OnMediaLocation & val_r ) { _location = val_r; return *this; }
      DeltaRpm & baseversion( const BaseVersion & val_r )          { _baseversion = val_r; return *this; }
      DeltaRpm & buildtime( const Date & val_r )                   { _buildtime = val_r; return *this; }

    private:
      source::OnMediaLocation _location;
      BaseVersion             _baseversion;
      Date                    _buildtime;
    };

    /** \relates DeltaRpm Stream output. */
    std::ostream & operator<<( std::ostream & str, const DeltaRpm & obj );

    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace packagedelta
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PACKAGEDELTA_H
