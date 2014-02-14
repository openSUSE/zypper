/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CpeId.h
 */
#ifndef ZYPP_CPEID_H
#define ZYPP_CPEID_H

#include <iosfwd>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/EnumClass.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class CpeId
  /// \brief Common Platform Enumearation
  /// See http://cpe.mitre.org/ for more information on the
  /// Common Platform Enumearation.
  ///////////////////////////////////////////////////////////////////
  class CpeId
  {
  public:
    struct NoThrowType {};
    static constexpr NoThrowType noThrow = NoThrowType();

  public:
    /** CPE name matching results (use like 'enum class \ref Match') */
    struct _MatchDef { enum Enum {
      undefined,
      equal,
      subset,
      superset,
      disjoint,
    };};
    typedef base::EnumClass<_MatchDef> Match;	///< 'enum class Match'

  public:
    /** Default ctor: empty Cpeid */
    CpeId();

    /** Ctor parsing from string
     * \throws std::invalid_argument if string is malformed
     */
    CpeId( const std::string & cpe_r );

    /** Ctor parsing from string (non throwing)
     * ICreates an empty CpeId if string is malformed.
     */
    CpeId( const std::string & cpe_r, NoThrowType );

    /** Dtor */
    ~CpeId();

  public:
    /**  Evaluate in boolean context: not an empty CpeId */
    explicit operator bool() const;

    /** String representation. */
    std::string asString() const;

  public:
    /** CPE name matching */
    Match match( const CpeId & rhs ) const;

  public:
    class Impl;                 ///< Implementation class.
  private:
    RWCOW_pointer<Impl> _pimpl; ///< Pointer to implementation.
  };

  /** \relates CpeId Stream output */
  inline std::ostream & operator<<( std::ostream & str, const CpeId & obj )
  { return str << obj.asString(); }

  /** \relates CpeId operator== based on string representation \b not matching. */
  inline bool operator==( const CpeId & lhs, const CpeId & rhs )
  { return lhs.asString() == rhs.asString(); }

  /** \relates CpeId operator!= based on string representation \b not matching. */
  inline bool operator!=( const CpeId & lhs, const CpeId & rhs )
  { return !(lhs == rhs); }

  /** \relates CpeId CPE name matching. */
  inline CpeId::Match match( const CpeId & lhs, const CpeId & rhs )
  { return lhs.match( rhs ); }


} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CPEID_H
