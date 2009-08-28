/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Rel.h
 *
*/
#ifndef ZYPP_REL_H
#define ZYPP_REL_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Rel
  //
  /** Relational operators.
   * Yes, it could as well be simply an \c enum.<BR>
   * Yes, you can use the relational operators as if it was an \c enum.<BR>
   * Except for use in a \c switch statement; see \ref inSwitch for this.
   *
   * But we want to construct them from a string representation, as well as
   * providing one. And this way they are wrapped into a namespace, which is
   * a good idea anyway.
   *
   * \ref ANY and \ref NONE are somewhat special. \ref ANY is the
   * operator created by the default ctor, and it should always resolve
   * to \c true. This may be handy in queries when you're looking for a
   * Resolvable in \c ANY Edition if no operator was specified.
   * While \ref NONE should always resolve to \c false.
   *
   * \ingroup g_EnumerationClass
  */
  struct Rel
  {
    /** \name Relational operators
     * These are the \em real relational operator contants to
     * use. Don't mind that it's not an enum. See also: \ref zypp::Rel::inSwitch
    */
    //@{
    static const Rel EQ;
    static const Rel NE;
    static const Rel LT;
    static const Rel LE;
    static const Rel GT;
    static const Rel GE;
    static const Rel ANY;
    static const Rel NONE;
    //@}

    /** Enumarators provided \b only for use \ref inSwitch statement.
     * \see inSwitch
     * \note Enumarator values also correspond to the values libsatsolver
     * uses to encode these relations.
    */
    enum for_use_in_switch {
      NONE_e = 0U,
      GT_e   = 1U,
      EQ_e   = 2U,
      LT_e   = 4U,
      GE_e   = GT_e|EQ_e,
      LE_e   = LT_e|EQ_e,
      NE_e   = GT_e|LT_e,
      ANY_e  = GT_e|EQ_e|LT_e,
    };

    /** DefaultCtor ANY. */
    Rel()
    : _op( ANY_e )
    {}

    /** Ctor from string.
     * Legal values for \a strval_r are: "==", "!=", "<", "<=", ">", ">=",<BR>
     * as well as "EQ", "NE", "LT", "LE", "GT", "GE", "ANY", "NONE"<BR>
     * and "" (empty string resolves to ANY).
     *
     * Lower case names are accepted as well.
     *
     * \throw PARSE if \a strval_r is not legal.
     * \todo refine exceptions and check throw.
     */
    explicit
    Rel( const std::string & strval_r );

    /** Ctor from string (non-throwing).
     * Illegal string values resolve to \c default_r
     */
    Rel( const std::string & strval_r, const Rel & default_r );

    /** Assign from string IFF it contains a legal value.
     * \return Whether \a strval_r contained a legal value.
    */
    bool parseFrom( const std::string & strval_r );

    /** Ctor from bits. */
    explicit
    Rel( unsigned bits_r )
    : _op( for_use_in_switch(bits_r & ANY_e) )
    {}

    /** Test whether \a bits_r is a valid \ref Rel (no extra bits set). */
    static bool isRel( unsigned bits_r )
    { return (bits_r & ANY_e) == bits_r; }

    /** String representation of relational operator.
     * \return "==", "!=", "<", "<=", ">", ">=", "ANY" or "NONE"
    */
    const std::string & asString() const;
    /** \overload */
    const char * c_str() const
    { return asString().c_str(); }

    /** Enumarator provided for use in \c switch statement.
     * The sole reason for providing enum \ref for_use_in_switch is,
     * that we may want to use the relational operators in a \c switch
     * statement. Tht's the only case where you should have to use the
     * enumarator.
     * \code
     *   Rel op;
     *   switch ( op.inSwitch() )
     *     {
     *     case Rel::EQ_e:
     *       ...
     *       break;
     *     case Rel::NE_e:
     *       ...
     *
     *     // No default! Let compiler warn if case is missing
     *     }
     * \endcode
    */
    for_use_in_switch inSwitch() const
    { return _op; }

    /** Enumarator values suitable for libsatsolver. */
    unsigned bits() const
    { return _op; }

  private:
    /** Ctor to initialize the relational operator contants. */
    Rel( for_use_in_switch op_r )
    : _op( op_r )
    {}
    /** The operator. */
    for_use_in_switch _op;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Rel Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Rel & obj )
  { return str << obj.asString(); }

  ///////////////////////////////////////////////////////////////////

  /** \relates Rel */
  inline bool operator==( const Rel & lhs, const Rel & rhs )
  { return lhs.inSwitch() == rhs.inSwitch(); }

  /** \relates Rel */
  inline bool operator!=( const Rel & lhs, const Rel & rhs )
  { return ! ( lhs == rhs ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REL_H
