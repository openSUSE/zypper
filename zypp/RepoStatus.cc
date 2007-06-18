/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RepoStatus.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"
#include "zypp/RepoStatus.h"
#include "zypp/PathInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus::Impl
  //
  /** RepoStatus implementation. */
  struct RepoStatus::Impl
  {

  public:
    
    string checksum;
    Date timestamp;
    
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoStatus::Impl Stream output */
  inline std::ostream & operator<<( std::ostream & str, const RepoStatus::Impl & obj )
  {
    return str << "RepoStatus::Impl";
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : RepoStatus
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoStatus::RepoStatus
  //	METHOD TYPE : Ctor
  //
  RepoStatus::RepoStatus()
  : _pimpl( new Impl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : RepoStatus::~RepoStatus
  //	METHOD TYPE : Dtor
  //
  RepoStatus::~RepoStatus()
  {}

  RepoStatus::RepoStatus( const Pathname &path )
  {
     _pimpl->checksum = filesystem::sha1sum(path);
  }
  
  RepoStatus & RepoStatus::setChecksum( const string &checksum )
  {
    _pimpl->checksum = checksum;
    return *this;
  }

  RepoStatus & RepoStatus::setTimestamp( const Date &timestamp )
  {
    _pimpl->timestamp = timestamp;
    return *this;
  }
  
  string RepoStatus::checksum() const
  { return _pimpl->checksum; }

  Date RepoStatus::timestamp() const
  { return _pimpl->timestamp; }
  
  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const RepoStatus & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
