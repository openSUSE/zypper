/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Edition.h
 *
*/
#ifndef ZYPP_EDITION_H
#define ZYPP_EDITION_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"

#include "zypp/Rel.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Edition
  //
  /** */
  class Edition
  {
  public:
    /** Type of an epoch. */
    typedef unsigned epoch_t;

  public:
    /** Default ctor. */
    Edition();
    /** Ctor taking \a version_r, \a release_r and optional \a epoch_r */
    Edition( const std::string & version_r,
             const std::string & release_r,
             epoch_t epoch_r = 0 );
    /** Dtor */
    ~Edition();

  public:
    /** */
    epoch_t epoch() const;
    /** */
    const std::string & version() const;
    /** */
    const std::string & release() const;

    /** String representation of Edition. */
    std::string asString() const;

  public:
    /** Compare Editions by relationam operator \a op.
     * \see Rel.
    */
    static bool compare( Rel op, const Edition & lhs, const Edition & rhs );

  private:
    /** Hides implementation */
    struct Impl;
    /** Pointer to implementation */
    base::ImplPtr<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Edition Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Edition & obj )
  { return str << obj.asString(); }

  /** \name Comaprison based on epoch, version, and release. */
  //@{

  /** \relates Edition */
  inline bool operator==( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::EQ, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator!=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::NE, lhs, rhs );; }

  /** \relates Edition */
  inline bool operator<( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::LT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator<=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::LE, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::GT, lhs, rhs ); }

  /** \relates Edition */
  inline bool operator>=( const Edition & lhs, const Edition & rhs )
  { return Edition::compare( Rel::GE, lhs, rhs ); }

  /** \relates Edition */
  inline int compare( const Edition & lhs, const Edition & rhs )
  { return lhs == rhs ? 0 : ( lhs < rhs ? -1 : 1 ); }

  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////
  /** \relates Edition Default order for std::container based on string value.*/
  template<>
    inline bool less<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() < rhs.asString(); }
  /** \relates Edition Equality for std::container classes based on string value. */
  template<>
    inline bool equal_to<zypp::Edition>::operator()( const zypp::Edition & lhs, const zypp::Edition & rhs ) const
    { return lhs.asString() == rhs.asString(); }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_EDITION_H
