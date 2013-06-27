/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/modalias/Modalias.h
 *
*/
#ifndef ZYPP_TARGET_MODALIAS_MODALIAS_H
#define ZYPP_TARGET_MODALIAS_MODALIAS_H

#include <iosfwd>
#include <vector>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/IdString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Modalias
    //
    /** Hardware abstaction layer singleton.
     */
    class Modalias
    {
      friend std::ostream & operator<<( std::ostream & str, const Modalias & obj );

      public:
        /** Implementation  */
        class Impl;

      public:
	typedef std::vector<std::string> ModaliasList;

        /** Singleton access. */
        static Modalias & instance();

        /** Dtor */
        ~Modalias();

      public:

        /** Checks if a device on the system matches a modalias pattern.
         *
         * Returns \c false if no matching device is found, and the modalias
         * of the first matching device otherwise. (More than one device
         * may match a given pattern.)
         *
         * On a system that has the following device,
         * \code
         *   pci:v00008086d0000265Asv00008086sd00004556bc0Csc03i00
         * \endcode
         * the following query will return \c true:
         * \code
         *   modalias_matches("pci:v00008086d0000265Asv*sd*bc*sc*i*")
         * \endcode
         */
        bool query( IdString cap_r ) const
        { return query( cap_r.c_str() ); }
        /** \overload */
        bool query( const char * cap_r ) const;
        /** \overload */
        bool query( const std::string & cap_r ) const
        { return query( cap_r.c_str() ); }

        /** List of modaliases found on system */
        const ModaliasList & modaliasList() const;

	/** Manually set list of modaliases to use */
	void modaliasList( ModaliasList newlist_r );

      private:
        /** Singleton ctor. */
        Modalias();

        /** Pointer to implementation */
        RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Modalias Stream output */
    std::ostream & operator<<( std::ostream & str, const Modalias & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_MODALIAS_MODALIAS_H
