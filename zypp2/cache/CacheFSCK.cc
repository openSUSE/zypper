/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp2/cache/CacheFSCK.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp2/cache/CacheFSCK.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

using namespace zypp;
using namespace zypp::cache;
using namespace std;
using namespace sqlite3x;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheFSCK::Impl
    //
    /** CacheFSCK implementation. */
    struct CacheFSCK::Impl
    {

    public:
      
      Impl( const Pathname &dbdir )
        : _dbdir(dbdir)
      {
  
      }
      
      void start()
      {
        try
        {
          sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
          //con.executenonquery("BEGIN;");

          sqlite3_command cmd( con, "PRAGMA integrity_check;");
          sqlite3_reader reader = cmd.executereader();
          while(reader.read())
          {
            cout << reader.getstring(0) << endl;
          }
        }
        catch( const std::exception &e )
        {
          ZYPP_RETHROW(Exception(e.what()));
        }
      }

    private:
      friend Impl * rwcowClone<Impl>( const Impl * rhs );
      /** clone for RWCOW_pointer */
      Impl * clone() const
      { return new Impl( *this ); }
      
      Pathname _dbdir;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CacheFSCK::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const CacheFSCK::Impl & obj )
    {
      return str << "CacheFSCK::Impl";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheFSCK
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheFSCK::CacheFSCK
    //	METHOD TYPE : Ctor
    //
    CacheFSCK::CacheFSCK( const Pathname &dbdir )
    : _pimpl( new Impl(dbdir) )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CacheFSCK::~CacheFSCK
    //	METHOD TYPE : Dtor
    //
    CacheFSCK::~CacheFSCK()
    {}

    void CacheFSCK::start()
    {
      _pimpl->start();
    }
    
    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const CacheFSCK & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp2
///////////////////////////////////////////////////////////////////
