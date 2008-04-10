/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/LocaleSupport.h
 *
*/
#ifndef ZYPP_SAT_LOCALESUPPORT_H
#define ZYPP_SAT_LOCALESUPPORT_H

#include <iosfwd>

#include "zypp/sat/detail/PoolMember.h"
#include "zypp/sat/SolvIterMixin.h"
#include "zypp/Locale.h"
#include "zypp/Filter.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : LocaleSupport
    //
    /** Convenience methods to manage support for a specific \ref Locale.
     *
     * \code
     *   sat::LocaleSupport myLocale( Locale("de") );
     *
     *   if ( myLocale.isAvailable() )
     *   {
     *     MIL << "Support for locale '" << myLocale.locale() << "' is available." << endl;
     *   }
     *   if ( ! myLocale.isRequested() )
     *   {
     *     MIL << "Will enable support for locale '" << myLocale.locale() << "'." << endl;
     *     myLocale.setRequested( true );
     *   }
     *   MIL << "Packages supporting locale '" << myLocale.locale() << "':" << endl;
     *   for_( it, myLocale.begin(), myLocale.end() )
     *   {
     *     // iterate over sat::Solvables
     *     MIL << "  " << *it << endl;
     *     // or get the PoolItems
     *     DBG << "  " << PoolItem(*it) << endl;
     *   }
     * \endcode
     *
     * \todo If iterator is too slow install a proxy watching the Pool::serial.
     */
    class LocaleSupport : public SolvIterMixin<LocaleSupport,filter_iterator<filter::ByLocaleSupport,Pool::SolvableIterator> >
                        , protected detail::PoolMember
    {
      public:
        /** Default ctor */
        LocaleSupport()
        {}
        /** Ctor taking a \ref Locale. */
        LocaleSupport( const Locale & locale_r )
        :  _locale( locale_r )
        {}

      public:
        /** My \ref Locale */
        const Locale & locale() const
        { return _locale; }

        /** Whether there are language specific packages supporting my \ref Locale. */
        bool isAvailable() const
        { return Pool(*this).isAvailableLocale( _locale ); }

        /** Whether the solver will automatically select language specific packages for my \ref Locale. */
        bool isRequested() const
        { return Pool(*this).isRequestedLocale( _locale ); }

        /** Turn on/off solver support for my \ref Locale.*/
        void setRequested( bool yesno_r )
        { yesno_r ? Pool(*this).addRequestedLocale( _locale ) : Pool(*this).eraseRequestedLocale( _locale ); }

      public:
        /** \name Iterate through all \ref sat::Solvables supporting my \ref Locale. */
        //@{
        typedef Solvable_iterator iterator;  // from SolvIterMixin

        iterator begin() const
        { return Pool(*this).filterBegin( filter::ByLocaleSupport( _locale ) ); }

        iterator end() const
        { return Pool(*this).filterEnd( filter::ByLocaleSupport( _locale ) ); }
        //@}

      private:
        Locale _locale;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates LocaleSupport Stream output */
    std::ostream & operator<<( std::ostream & str, const LocaleSupport & obj );

    /** \relates LocaleSupport More verbose stream output including dependencies */
    std::ostream & dumpOn( std::ostream & str, const LocaleSupport & obj );

    /** \relates LocaleSupport */
    inline bool operator==( const LocaleSupport & lhs, const LocaleSupport & rhs )
    { return lhs.locale() == rhs.locale(); }

    /** \relates LocaleSupport */
    inline bool operator!=( const LocaleSupport & lhs, const LocaleSupport & rhs )
    { return lhs.locale() != rhs.locale(); }

    /** \relates LocaleSupport */
    inline bool operator<( const LocaleSupport & lhs, const LocaleSupport & rhs )
    { return lhs.locale() < rhs.locale(); }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_LOCALESUPPORT_H
