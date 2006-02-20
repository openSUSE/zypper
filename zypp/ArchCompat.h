/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ArchCompat.h
 *
*/
#ifndef ZYPP_ARCHCOMPAT_H
#define ZYPP_ARCHCOMPAT_H

#include <iosfwd>
#include <map>
#include <string>

#include <zypp/Bit.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace arch
  { /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Compat
  //
  /** Define architecture compatibility (Singleton).
   *
   * On the fly unify architecture strings used by
   * \ref zypp::Arch. Actually an implementaion detail,
   * exposed to allow a \ref zypp::Arch to store an iterator
   * into the CompatMap instead of an additional Impl_ptr,
   * or doing lookups by string all the time.
   *
  */
  class Compat
  {
  public:
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Compat::Entry
    //
    /** Holds an architecture ID and it's compatibleWith relation.
     * An architecture is compatibleWith, if it's _idBit is set in
     * _compatBits.
    */
    struct Entry
    {
      /** Bitfield for architecture IDs and compatibleWith relation. */
      typedef bit::BitField<uint16_t> CompatBits;

      friend std::ostream & operator<<( std::ostream & str, const Entry & obj )
      { return str << obj._idBit << ' ' << obj._compatBits; }

      /** Return whether \c this is compatible with \a compat_r.*/
      bool compatibleWith( const Entry & compat_r ) const
      {
        // The test '== compat_r._idBit' is required!
        // noarch and non builtin archs have <tt>_idBit == 0</tt>
#warning FIX nonbuiltin need _idBit 1 and spacial handling
        return (_compatBits & compat_r._idBit) == compat_r._idBit;
      }

      /** Ctor. Default to _idBit 1 for nonbuiltin archs.*/
      Entry( CompatBits::IntT idBit_r = CompatBits::IntT(1) )
      : _idBit( idBit_r )
      , _compatBits( idBit_r )
      {}

    private:
      friend class ComaptMapImpl;
      CompatBits _idBit;
      CompatBits _compatBits;
    };
    ///////////////////////////////////////////////////////////////////

    /** The CompatMap. */
    typedef std::map<std::string, Entry> CompatMap;
    typedef CompatMap::iterator          iterator;
    typedef CompatMap::const_iterator    const_iterator;

  public:
    /** Singleton access. */
    static Compat instance();

    const_iterator begin() const;
    const_iterator end() const;

    /** Return the CompatMap entry related to \a archStr_r. */
    const_iterator assertDef( const std::string & archStr_r );
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Compat Stream output */
  std::ostream & operator<<( std::ostream & str, const Compat & obj );

  /////////////////////////////////////////////////////////////////
  } // namespace arch
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ARCHCOMPAT_H
