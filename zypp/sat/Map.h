/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Map.h
 */
#ifndef ZYPP_SAT_MAP_H
#define ZYPP_SAT_MAP_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/sat/detail/PoolMember.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace sat
  {
    ///////////////////////////////////////////////////////////////////
    /// \class Map
    /// \brief Libsolv (bit)Map wrapper.
    ///
    /// \Note Requested sizes are filled up to the next multiple of eight.
    /// Libsolv bitmaps are not shrinkable.
    ///////////////////////////////////////////////////////////////////
    class Map
    {
    public:
      typedef unsigned long size_type;

      /** Type to indicate the bitmap should match the current pools capacity. */
      struct PoolSizeType {};
      /** An object indicating the bitmap should match the current pools capacity. */
      static constexpr PoolSizeType poolSize = PoolSizeType();

    public:
      /** Default ctor: empty Map */
      Map();

      /** Ctor taking the Map size */
      explicit Map( size_type size_r );

      /** Ctor creating a Map matching the current pools capacity */
      Map( PoolSizeType );

      /** Dtor */
      ~Map();

    public:
      /** Whether Map is empty. */
      bool empty() const;

      /** Size of the Map. */
      size_type size() const;

      /** Grow the Map if necessary. */
      void grow( size_type size_r );

    public:
      /** Set all bits. */
      void setAll();

      /** Clear all bits. */
      void clearAll();

      /** Assign \c val_r to all bits. */
      void assignAll( bool val_r );

      /** Set bit \c idx_r.
       * \throws std::out_of_range if \a idx_r is out of range
       */
      void set( size_type idx_r );

      /** Clear bit \c idx_r.
       * \throws std::out_of_range if \a idx_r is out of range
       */
      void clear( size_type idx_r );

      /** Assign \c val_r to bit \c idx_r.
       * \throws std::out_of_range if \a idx_r is out of range
       */
      void assign( size_type idx_r, bool val_r );

    public:
      /** Test bit \c idx_r.
       * \throws std::out_of_range if \a idx_r is out of range
       */
      bool test( size_type idx_r ) const;

      /** Test bit \c idx_r.
       * \throws std::out_of_range if \a idx_r is out of range
       */
      bool operator[]( size_type idx_r ) const
      { return test( idx_r ); }

    public:
      /** String representation */
      std::string asString( const char on_r = '1', const char off_r = '0' ) const;

    public:
      operator detail::CMap *();		///< libsolv backdoor
      operator const detail::CMap *() const	///< libsolv backdoor
      { return _pimpl.get(); }
    private:
      RWCOW_pointer<detail::CMap> _pimpl;	///< Pointer to implementation
    };

    /** \relates Map Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Map & obj )
    { return str << obj.asString(); }

    /** \relates Map */
    bool operator==( const Map & lhs, const Map & rhs );

    /** \relates Map */
    inline bool operator!=( const Map & lhs, const Map & rhs )
    { return !( lhs == rhs ); }

  } // namespace sat
  ///////////////////////////////////////////////////////////////////

  /** \relates Map Clone function for RWCOW_pointer */
  template<> sat::detail::CMap * rwcowClone<sat::detail::CMap>( const sat::detail::CMap * rhs );

  typedef sat::Map Bitmap;

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_MAP_H
