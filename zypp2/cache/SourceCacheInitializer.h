/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SourceCacheInitializer_H
#define ZYPP_SourceCacheInitializer_H

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

    DEFINE_PTR_TYPE(SourceCacheInitializer);

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SourceCacheInitializer
    //
    class SourceCacheInitializer : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SourceCacheInitializer & obj );

    public:
      /**
       * Tries to initialize the source cache if it was not
       * \throws When cant initialize
       */
      SourceCacheInitializer( const Pathname &root_r, const Pathname &db_file );
      ~SourceCacheInitializer();

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

    /** \relates SourceCacheInitializer Stream output */
    inline std::ostream & operator<<( std::ostream & str, const SourceCacheInitializer & obj )
    { return obj.dumpOn( str ); }


    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SourceCacheInitializer_H
