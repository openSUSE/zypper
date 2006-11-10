/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Arch.h
 *
*/
#ifndef ZYPP_ARCH_H
#define ZYPP_ARCH_H

#include <iosfwd>
#include <functional>
#include <set>
#include <string>

#include "zypp/RelCompare.h"
#include "zypp/base/String.h"
#include "zypp/base/Iterator.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Arch
  //
  /** Architecture.
  */
  class Arch
  {
  public:
    /** Default ctor 'noarch' */
    Arch();
    /** Ctor from string. */
    explicit
    Arch( const std::string & rhs );

  public:
    /** String representation of Arch. */
    const std::string & asString() const;

    /** Test for an empty Arch (this is "", not Arch_noarch). */
    bool empty() const
    { return asString().empty(); }

    /** Compatibility relation.
     * \return \c True iff \c this is compatible with \a targetArch_r.
     * \code
     * Arch_noarch.compatibleWith( ... )       ==> always true;
     * Arch_i686.compatibleWith( Arch_x86_64 ) ==> true;
     * Arch_x86_64.compatibleWith( Arch_i686 ) ==> false;
     * \endcode
    */
    bool compatibleWith( const Arch & targetArch_r ) const;

    /** Arch comparison.
     * Primary key is the number of compatible Archs, then
     * the string representation. Thus Arch_noarch is the
     * least Arch.
    */
    int compare( const Arch & rhs ) const;

    /** Arch comparison (static version). */
    static int compare( const Arch & lhs, const Arch & rhs )
    { return lhs.compare( rhs ); }

  public:
    /** Reversed arch order, best Arch first. */
    typedef std::set<Arch,CompareByGT<Arch> > CompatSet;

    /** Return a set of all Arch's \ref compatibleWith a \a targetArch_r.
     * \note The set is ordered according to compare, thus iterating
     * will start at \a targetArch_r and end with \c Arch_noarch.
     * \code
     * Arch::CompatSet cset( Arch::compatSet( Arch_x86_64 ) );
     *
     * cout << str::join( make_transform_iterator( cset.begin(), std::mem_fun_ref(&Arch::asString) ),
     *                    make_transform_iterator( cset.end(), std::mem_fun_ref(&Arch::asString) ) )
     *      << endl;
     *
     * // Prints: x86_64 athlon i686 i586 i486 i386 noarch
     * \endcode
    */
    static CompatSet compatSet( const Arch & targetArch_r );

    /** */
    static std::string asString( const CompatSet & cset )
    {
      return str::join( make_transform_iterator( cset.begin(), std::mem_fun_ref(&Arch::asString) ),
                        make_transform_iterator( cset.end(), std::mem_fun_ref(&Arch::asString) ) );
    }

  public:
    struct CompatEntry;
  private:
    Arch( const CompatEntry & );
    const CompatEntry * _entry;
  };
  ///////////////////////////////////////////////////////////////////

  /** \name Builtin architecture constants.
   *
   * Defined outside Arch as e.g. \c Arch_i386, because some names,
   * like \c i388, are used as \c #define, thus unusable as identifier
   * like \c Arch::i386.
  */
  //@{
  /** \relates Arch */
  extern const Arch Arch_noarch;

  /** \relates Arch */
  extern const Arch Arch_x86_64;
  /** \relates Arch */
  extern const Arch Arch_athlon;
  /** \relates Arch */
  extern const Arch Arch_i686;
  /** \relates Arch */
  extern const Arch Arch_i586;
  /** \relates Arch */
  extern const Arch Arch_i486;
  /** \relates Arch */
  extern const Arch Arch_i386;

  /** \relates Arch */
  extern const Arch Arch_s390x;
  /** \relates Arch */
  extern const Arch Arch_s390;

  /** \relates Arch */
  extern const Arch Arch_ppc64;
  /** \relates Arch */
  extern const Arch Arch_ppc;

  /** \relates Arch */
  extern const Arch Arch_ia64;
  //@}

  ///////////////////////////////////////////////////////////////////

  /** \relates Arch stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Arch & obj )
  { return str << obj.asString(); }

  /** \name Equality based on string value. */
  //@{
  /** \relates Arch */
  inline bool operator==( const Arch & lhs, const Arch & rhs )
  { return lhs.asString() == rhs.asString(); }

  /** \relates Arch */
  inline bool operator==( const Arch & lhs, const std::string & rhs )
  { return lhs.asString() == rhs; }

  /** \relates Arch */
  inline bool operator==( const std::string & lhs, const Arch & rhs )
  { return lhs == rhs.asString(); }

  /** \relates Arch */
  inline bool operator!=( const Arch & lhs, const Arch & rhs )
  { return !( lhs == rhs ); }

  /** \relates Arch */
  inline bool operator!=( const Arch & lhs, const std::string & rhs )
  { return !( lhs == rhs ); }

  /** \relates Arch */
  inline bool operator!=( const std::string & lhs, const Arch & rhs )
  { return !( lhs == rhs ); }
  //@}

  ///////////////////////////////////////////////////////////////////

  /** Functor finding compatible architectures.
   * \see Arch::compatibleWith
  */
  struct ArchCompatibleWith : public std::unary_function<Arch,bool>
  {
    /** The target architecture */
    Arch _targetArch;
    /** Ctor taking the target architecture */
    ArchCompatibleWith( const Arch & targetArch_r )
    : _targetArch( targetArch_r )
    {}
    /** Call Arch::compatibleWith ( \c _targetArch ) on \a rhs. */
    bool operator()( const Arch & rhs ) const
    { return rhs.compatibleWith( _targetArch ); }
  };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////
  /** \relates zypp::Arch Default order for std::container based Arch::compare.*/
  template<>
    inline bool less<zypp::Arch>::operator()( const zypp::Arch & lhs, const zypp::Arch & rhs ) const
    { return lhs.compare( rhs ) < 0; }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ARCH_H
