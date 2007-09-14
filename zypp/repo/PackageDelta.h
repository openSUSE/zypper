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

#include "zypp/OnMediaLocation.h"
#include "zypp/Edition.h"
#include "zypp/Arch.h"
#include "zypp/Date.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace packagedelta
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////

    /** \todo cheap copy! (switch to RWCOW) */
    class PatchRpm
    {
    public:
      typedef Edition                BaseVersion;
      typedef std::list<BaseVersion> BaseVersions;

    public:
      PatchRpm()
      {}

    public:
      /** \name Target package ident. */
      //@{
      const std::string &     name()         const { return _name; }
      const Edition &         edition()      const { return _edition; }
      const Arch &            arch()         const { return _arch; }
      //@}
      const OnMediaLocation & location()     const { return _location; }
      const BaseVersions &    baseversions() const { return _baseversions; }
      const Date &            buildtime()    const { return _buildtime;}

    public:
      PatchRpm & setName( const std::string & val_r )         { _name = val_r; return *this; }
      PatchRpm & setEdition( const Edition & val_r )          { _edition = val_r; return *this; }
      PatchRpm & setArch( const Arch & val_r )                { _arch = val_r; return *this; }
      PatchRpm & setLocation( const OnMediaLocation & val_r ) { _location = val_r; return *this; }
      PatchRpm & setBaseversions( const BaseVersions & val_r ){ _baseversions = val_r; return *this; }
      PatchRpm & addBaseversion( const BaseVersion & val_r )  { _baseversions.push_back( val_r ); return *this; }
      PatchRpm & setBuildtime( const Date & val_r )           { _buildtime = val_r; return *this; }

    private:
      std::string     _name;
      Edition         _edition;
      Arch            _arch;
      OnMediaLocation _location;
      BaseVersions    _baseversions;
      Date            _buildtime;
    };

    /** \relates PatchRpm Stream output. */
    std::ostream & operator<<( std::ostream & str, const PatchRpm & obj );

    ///////////////////////////////////////////////////////////////////

    /** \todo cheap copy! (switch to RWCOW) */
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
        BaseVersion & setEdition( const Edition & val_r )          { _edition = val_r; return *this; }
        BaseVersion & setBuildtime( const Date & val_r )           { _buildtime = val_r; return *this; }
        BaseVersion & setChecksum( const CheckSum & val_r )        { _checksum = val_r; return *this; }
        BaseVersion & setSequenceinfo( const std::string & val_r ) { _sequenceinfo = val_r; return *this; }

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
      /** \name Target package ident. */
      //@{
      const std::string &     name()         const { return _name; }
      const Edition &         edition()      const { return _edition; }
      const Arch &            arch()         const { return _arch; }
      //@}
      const OnMediaLocation & location()     const { return _location; }
      const BaseVersion &     baseversion()  const { return _baseversion; }
      const Date &            buildtime()    const { return _buildtime;}

    public:
      DeltaRpm & setName( const std::string & val_r )         { _name = val_r; return *this; }
      DeltaRpm & setEdition( const Edition & val_r )          { _edition = val_r; return *this; }
      DeltaRpm & setArch( const Arch & val_r )                { _arch = val_r; return *this; }
      DeltaRpm & setLocation( const OnMediaLocation & val_r ) { _location = val_r; return *this; }
      DeltaRpm & setBaseversion( const BaseVersion & val_r )  { _baseversion = val_r; return *this; }
      DeltaRpm & setBuildtime( const Date & val_r )           { _buildtime = val_r; return *this; }

    private:
      std::string     _name;
      Edition         _edition;
      Arch            _arch;
      OnMediaLocation _location;
      BaseVersion     _baseversion;
      Date            _buildtime;
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
