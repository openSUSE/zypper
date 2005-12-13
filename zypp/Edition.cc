/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "base/String.h"
#include "base/Exception.h"

#include "zypp/Edition.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	hidden details
  //
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    /** Rpm version comparison.
     * \a lhs and \a rhs are expected to be either version or release
     * strings. Not both separated by a '-'.
     * \return <tt>-1,0,1</tt> if version strings are <tt>\<,==,\></tt>
     * \todo review
    */
    int rpmverscmp( const std::string & lhs, const std::string & rhs )
    {
      int  num1, num2;
      char oldch1, oldch2;
      char * str1, * str2;
      char * one, * two;
      int  rc;
      int  isnum;

      // equal?
      if ( lhs == rhs )  return 0;

      // empty is less than anything else:
      if ( lhs.empty() ) return -1;
      if ( rhs.empty() ) return  1;

      str1 = (char*)alloca( lhs.size() + 1 );
      str2 = (char*)alloca( rhs.size() + 1 );

      strcpy( str1, lhs.c_str() );
      strcpy( str2, rhs.c_str() );

      one = str1;
      two = str2;

      // split strings into segments of alpha or digit
      // sequences and compare them accordingly.
      while ( *one && *two ) {

        // skip non alphanumerical chars
        while ( *one && ! isalnum( *one ) ) ++one;
        while ( *two && ! isalnum( *two ) ) ++two;
        if ( ! ( *one && *two ) )
          break; // reached end of string

        // remember segment start
        str1 = one;
        str2 = two;

        // jump over segment, type determined by str1
        if ( isdigit( *str1 ) ) {
          while ( isdigit( *str1 ) ) ++str1;
          while ( isdigit( *str2 ) ) ++str2;
          isnum = 1;
        } else {
          while ( isalpha( *str1 ) ) ++str1;
          while ( isalpha( *str2 ) ) ++str2;
          isnum = 0;
        }

        // one == str1 -> can't be as strings are not empty
        // two == str2 -> mixed segment types
        if ( two == str2 ) return( isnum ? 1 : -1 );

        // compare according to segment type
        if ( isnum ) {
          // atoi() may overflow on long segments
          // skip leading zeros
          while ( *one == '0' ) ++one;
          while ( *two == '0' ) ++two;
          // compare number of digits
          num1 = str1 - one;
          num2 = str2 - two;
          if ( num1 != num2 ) return( num1 < num2 ? -1 : 1 );
        }

        // strcmp() compares alpha AND equal sized number segments
        // temp. \0-terminate segment
        oldch1 = *str1;
        *str1 = '\0';
        oldch2 = *str2;
        *str2 = '\0';

        rc = strcmp( one, two );
        if ( rc ) return rc;

        // restore original strings
        *str1 = oldch1;
        *str2 = oldch2;

        // prepare for next cycle
        one = str1;
        two = str2;
      }

      // check which strings are now empty
      if ( !*one ) {
        return( !*two ? 0 : -1 );
      }
      return 1;
    }
  }
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition::Impl
  //
  /** Edition implementation.
   * \todo Unifiy Impl in Edition::noedition and Edition::Edition()
  */
  struct Edition::Impl
  {
    Impl()
    : _epoch( noepoch )
    {}

    Impl( const std::string & edition_r )
    : _epoch( noepoch )
    {
      str::smatch what;
      if( str::regex_match( edition_r.begin(), edition_r.end(),
                            what, _rxEdition ) )
        {
          // what[2] contains the epoch
          // what[3] contains the version
          // what[5] contains the release
          if ( what[2].matched )
            _epoch = strtoul( what[2].str().c_str(), NULL, 10 );
          if ( what[3].matched )
            _version = what[3].str();
          if ( what[5].matched )
            _release = what[5].str();
        }
      else
        {
          ZYPP_THROW( Exception(string("Invalid Edition: ")+edition_r) );
        }
    }

    Impl( const std::string & version_r,
          const std::string & release_r,
          epoch_t epoch_r )
    : _epoch( epoch_r )
    , _version( validateVR(version_r) )
    , _release( validateVR(release_r) )
    {}

    Impl( const std::string & version_r,
          const std::string & release_r,
          const std::string & epoch_r )
    : _epoch( validateE(epoch_r) )
    , _version( validateVR(version_r) )
    , _release( validateVR(release_r) )
    {}

    /** Dtor */
    ~Impl()
    {}

    /** return validated epoch ([0-9]*) or throw */
    static epoch_t validateE( const std::string & epoch_r )
    {
      if ( epoch_r.empty() )
        return noepoch;

      char * endptr = NULL;
      epoch_t ret = strtoul( epoch_r.c_str(), &endptr, 10 );
      if ( *endptr != '\0' )
        ZYPP_THROW( Exception(string("Invalid eopch: ")+epoch_r) );
      return ret;
    }

    /** return validated version/release or throw */
    static const std::string & validateVR( const std::string & vr_r )
    {
      str::smatch what;
      if( ! str::regex_match( vr_r.begin(), vr_r.end(), what, _rxVR ) )
        ZYPP_THROW( Exception(string("Invalid version/release: ")+vr_r) );
      return vr_r;
    }

    epoch_t      _epoch;
    std::string _version;
    std::string _release;

    static const str::regex _rxVR;
    static const str::regex _rxEdition;
  };
  ///////////////////////////////////////////////////////////////////

  const str::regex Edition::Impl::_rxVR( "([^-]*)" );

  const str::regex Edition::Impl::_rxEdition( "(([0-9]+):)?([^-]*)(-([^-]*))?" );

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition
  //
  ///////////////////////////////////////////////////////////////////

  const Edition Edition::noedition;

  ///////////////////////////////////////////////////////////////////

  Edition::Edition()
  : _pimpl( new Impl )
  {}

  Edition::Edition( const std::string & edition_r )
  : _pimpl( new Impl( edition_r ) )
  {}

  Edition::Edition( const std::string & version_r,
                    const std::string & release_r,
                    epoch_t epoch_r )
  : _pimpl( new Impl( version_r, release_r, epoch_r ) )
  {}

  Edition::Edition( const std::string & version_r,
                    const std::string & release_r,
                    const std::string & epoch_r )
  : _pimpl( new Impl( version_r, release_r, epoch_r ) )
  {}

  Edition::~Edition()
  {}

  Edition::epoch_t Edition::epoch() const
  { return _pimpl->_epoch; }

  const std::string & Edition::version() const
  { return _pimpl->_version; }

  const std::string & Edition::release() const
  { return _pimpl->_release; }

  std::string Edition::asString() const
  {
    string ret;

    if ( _pimpl->_epoch )
      ret += str::form(  "%d:", _pimpl->_epoch );

    ret += _pimpl->_version;

    if ( ! _pimpl->_release.empty() )
      {
        ret += '-';
        ret += _pimpl->_release;
      }

    if ( ret.empty() )
      return "EDITION-UNSPEC";

    return ret;
  }

  bool Edition::compare( Rel op, const Edition & lhs, const Edition & rhs )
  {
    switch ( op.inSwitch() )
      {
      case Rel::EQ_e:
        return compare( lhs, rhs ) == 0;
        break;
      case Rel::NE_e:
        return compare( lhs, rhs ) != 0;
        break;
      case Rel::LT_e:
        return compare( lhs, rhs ) == -1;
        break;
      case Rel::LE_e:
        return compare( lhs, rhs ) != 1;
        break;
      case Rel::GT_e:
        return compare( lhs, rhs ) == 1;
        break;
      case Rel::GE_e:
        return compare( lhs, rhs ) != -1;
        break;
      case Rel::ANY_e:
        return true;
        break;
      case Rel::NONE_e:
        return false;
        break;
      }
    // We shouldn't get here.
    INT << "Unknown relational opertor '" << op << "' treated as  'NONE'" << endl;
    return false;
  }

  int Edition::compare( const Edition & lhs, const Edition & rhs )
  {
    // compare epoch
    if ( lhs.epoch() != rhs.epoch() )
      return lhs.epoch() < rhs.epoch() ? -1 : 1;

    // next compare versions
    int res = rpmverscmp( lhs.version(), rhs.version() );
    if ( res )
      return res; // -1|1: not equal

    // finaly compare releases
    return rpmverscmp( lhs.release(), rhs.release() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
