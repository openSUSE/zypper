/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_CacheInitializer_H
#define ZYPP_CacheInitializer_H

#include <iosfwd>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(CacheInitializer);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CacheInitializer
    //
    class CacheInitializer : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const CacheInitializer & obj );

    public:
      /**
       * Tries to initialize the source cache if it was not
       * \throws When cant initialize
       */
      CacheInitializer( const Pathname &root_r, const Pathname &db_file );
      ~CacheInitializer();

      /**
       * only true when cache was not initialized before
       * and was just initialized with success
       */
      bool justInitialized() const;
    protected:
      bool tablesCreated() const;
			void createTables();
      /** Overload to realize stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      //typedef std::map<media::MediaNr, media::MediaAccessId> MediaMap
      shared_ptr<sqlite3x::sqlite3_connection> _con;
      Pathname _root;
      bool _just_initialized;
      
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CacheInitializer Stream output */
    inline std::ostream & operator<<( std::ostream & str, const CacheInitializer & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_CacheInitializer_H
