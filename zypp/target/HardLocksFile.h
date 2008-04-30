/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/HardLocksFile.h
 *
*/
#ifndef ZYPP_TARGET_HARDLOCKSFILE_H
#define ZYPP_TARGET_HARDLOCKSFILE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/Pathname.h"
#include "zypp/pool/PoolTraits.h"
#include "zypp/PoolQuery.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : HardLocksFile
    //
    /** Save and restore hardlocks.
     */
    class HardLocksFile
    {
      friend std::ostream & operator<<( std::ostream & str, const HardLocksFile & obj );
      public:

        typedef pool::PoolTraits::HardLockQueries Data;

      public:
        /** Ctor taking the file to read/write. */
        HardLocksFile( const Pathname & file_r )
        : _file( file_r )
        {}

        /** Return the file path. */
        const Pathname & file() const
        { return _file; }

        /** Return the data.
         * The file is read once on demand. Returns empty \ref Data if
         * the file does not exist or is not readable.
        */
        const Data & data() const
        {
          if ( !_dataPtr )
          {
            _dataPtr.reset( new Data );
            Data & mydata( *_dataPtr );
            load( _file, mydata );
          }
          return *_dataPtr;
        }

        /** Store new \ref Data.
         * Write the new \ref Data to file, unless we know it
         * did not change. The directory containing file must
         * exist.
        */
        void setData( const Data & data_r )
        {
          if ( !_dataPtr )
            _dataPtr.reset( new Data );

          if ( differs( *_dataPtr, data_r ) )
          {
            store( _file, data_r );
            *_dataPtr = data_r;
          }
        }

      private:
        /** Helper testing whether two \ref Data differ. */
        bool differs( const Data & lhs, const Data & rhs ) const
        {
          if ( lhs.size() != rhs.size() )
            return true;
          // Complete diff is too expensive and not necessary here.
          // Just check for the same sequence of items.
          Data::const_iterator rit = rhs.begin();
          for_( it, lhs.begin(), lhs.end() )
          {
            if ( *it != *rit )
              return true;
            ++rit;
          }
          return false;
        }
        /** Read \ref Data from \c file_r. */
        static void load( const Pathname & file_r, Data & data_r );
        /** Write \ref Data to \c file_r. */
        static void store( const Pathname & file_r, const Data & data_r );

      private:
        Pathname                 _file;
        mutable scoped_ptr<Data> _dataPtr;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates HardLocksFile Stream output */
    std::ostream & operator<<( std::ostream & str, const HardLocksFile & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_HARDLOCKSFILE_H
