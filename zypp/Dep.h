/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dep.h
 *
*/
#ifndef ZYPP_DEP_H
#define ZYPP_DEP_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dep
  //
  /** Enumeration class of dependency types.
   * \ingroup g_EnumerationClass
  */
  struct Dep
  {
    friend bool operator==( const Dep & lhs, const Dep & rhs );
    friend bool operator!=( const Dep & lhs, const Dep & rhs );
    /** Arbitrary order to allow Dep as key in std::container. */
    friend bool operator<( const Dep & lhs, const Dep & rhs );

    /** \name Dependency types
     * These are the \em real dependency type contants to
     * use. Don't mind that it's not an enum.
     * \see \ref zypp::Dep::inSwitch
    */
    //@{
    static const Dep PROVIDES;
    static const Dep PREREQUIRES;
    static const Dep REQUIRES;
    static const Dep CONFLICTS;
    static const Dep OBSOLETES;
    static const Dep RECOMMENDS;
    static const Dep SUGGESTS;
    static const Dep ENHANCES;
    static const Dep SUPPLEMENTS;
    //@}

    /** Enumarators provided \b only for use \ref inSwitch statement.
     * \see inSwitch
    */
    enum for_use_in_switch {
      PROVIDES_e,
      PREREQUIRES_e,
      REQUIRES_e,
      CONFLICTS_e,
      OBSOLETES_e,
      RECOMMENDS_e,
      SUGGESTS_e,
      ENHANCES_e,
      SUPPLEMENTS_e,
    };

    /** Ctor from string.
     * Legal values for \a strval_r are the constants names
     * (case insignificant).
     *
     * \throw PARSE if \a strval_r is not legal.
     * \todo refine exceptions and check throw.
    */
    explicit
    Dep( const std::string & strval_r );

    /** String representation of dependency type.
     * \return The constants names lowercased.
    */
    const std::string & asString() const;

    /** Translated dependency type (capitalized).
     * \return The capitalized constants names translated.
    */
    std::string asUserString() const;

    /** Enumarator provided for use in \c switch statement. */
    for_use_in_switch inSwitch() const
    { return _type; }

  private:
    /** Ctor to initialize the dependency type contants. */
    Dep( for_use_in_switch type_r )
    : _type( type_r )
    {}
    /** The operator. */
    for_use_in_switch _type;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Dep Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Dep & obj )
  { return str << obj.asString(); }

  ///////////////////////////////////////////////////////////////////

  /** \relates Dep */
  inline bool operator==( const Dep & lhs, const Dep & rhs )
  { return lhs._type == rhs._type; }

  /** \relates Dep */
  inline bool operator!=( const Dep & lhs, const Dep & rhs )
  { return lhs._type != rhs._type; }

  /** \relates Dep */
  inline bool operator<( const Dep & lhs, const Dep & rhs )
  { return lhs._type < rhs._type; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DEP_H
