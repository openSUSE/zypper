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
  /** Edition
   \todo doc
   \todo optimize implementation
   \todo implement debian comparison and make choice backend specific
  */
  class Edition
  {
  public:
    /** Type of an epoch. */
    typedef unsigned epoch_t;

    /** Value representing \c noepoch. */
    static const epoch_t noepoch = 0;

  public:
    /** Default ctor. */
    Edition();
    /** Ctor taking \a version_r, \a release_r and optional \a epoch_r */
    Edition( const std::string & version_r,
             const std::string & release_r,
             epoch_t epoch_r = noepoch );
    /** Dtor */
    ~Edition();

  public:
    /** Epoch */
    epoch_t epoch() const;
    /** Version */
    const std::string & version() const;
    /** Release */
    const std::string & release() const;

    /** String representation of Edition. */
    std::string asString() const;

  public:
    /** Compare two Editions using relational operator \a op.
     * \return Result of expression \c ( \a lhs \a op \a rhs \c ).<BR>
     * If \a op is Rel::ANY, the expression is always \c true.<BR>
     * If \a op is Rel::NONE, the expression is always \c false.
     *
     * \todo optimize impementation. currently a full compare( lhs, rhs )
     * is done and the result evaluated. But step by step would be faster.
    */
    static bool compare( Rel op, const Edition & lhs, const Edition & rhs );

    /** Compare two Editions returning <tt>-1,0,1</tt>.
     * \return <tt>-1,0,1</tt> if editions are <tt>\<,==,\></tt>
    */
    static int compare( const Edition & lhs, const Edition & rhs );

  private:
    /** Hides implementation */
    struct Impl;
    /** Pointer to implementation */
    base::RW_pointer<Impl> _pimpl;
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
