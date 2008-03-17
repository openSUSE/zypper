/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/RequestedLocalesFile.h
 *
*/
#ifndef ZYPP_TARGET_REQUESTEDLOCALESFILE_H
#define ZYPP_TARGET_REQUESTEDLOCALESFILE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/Pathname.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RequestedLocalesFile
    //
    /** Save and restore locale set from file.
     */
    class RequestedLocalesFile
    {
      friend std::ostream & operator<<( std::ostream & str, const RequestedLocalesFile & obj );

      public:
        /** Ctor taking the file to read/write. */
        RequestedLocalesFile( const Pathname & file_r )
        : _file( file_r )
        {}

        /** Return the file path. */
        const Pathname & file() const
        { return _file; }

        /** Return the loacale set.
         * The file is read once on demand. returns an empty set if
         * the file does not exist or is not readable.
        */
        const LocaleSet & locales() const
        {
          if ( !_localesPtr )
          {
            _localesPtr.reset( new LocaleSet );
            LocaleSet & ls( *_localesPtr );
            load( _file, ls );
          }
          return *_localesPtr;
        }

        /** Store a new locale set.
         * Write the new localeset to file, unless we know it
         * did not change. The directory containing file must exist.
        */
        void setLocales( const LocaleSet & locales_r )
        {
          if ( !_localesPtr )
            _localesPtr.reset( new LocaleSet );

          if ( differs( *_localesPtr, locales_r ) )
          {
            store( _file, locales_r );
            *_localesPtr = locales_r;
          }
        }

      private:
        /** Helper testing whether two \ref LocaleSet differ. */
        bool differs( const LocaleSet & lhs, const LocaleSet & rhs ) const
        {
          if ( lhs.size() != rhs.size() )
            return true;
          for_( it, lhs.begin(), lhs.end() )
          {
            if ( rhs.find( *it ) == rhs.end() )
              return true;
          }
          return false;
        }
        /** Read \ref LocaleSet from \c file_r. */
        static void load( const Pathname & file_r, LocaleSet & locales_r );
        /** Write \ref LocaleSet to \c file_r. */
        static void store( const Pathname & file_r, const LocaleSet & locales_r );

      private:
        Pathname                      _file;
        mutable scoped_ptr<LocaleSet> _localesPtr;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates RequestedLocalesFile Stream output */
    std::ostream & operator<<( std::ostream & str, const RequestedLocalesFile & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_REQUESTEDLOCALESFILE_H
