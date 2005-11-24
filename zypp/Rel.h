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
   * providing one. Anf this way they are wrapped into a namespace, which is
   * a good idea anyway.
   *
   * \ref ANY and \ref NONE are somewhat special. \ref NONE is the
   * operator created by the default ctor, and it should always resolve
   * to \c false. While \ref ANY should always resolve to \c true, and
   * may be handy in queries when you're looking for a Resolvable in
   * \c ANY Edition.
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
    */
    enum for_use_in_switch { EQ_e, NE_e, LT_e, LE_e, GT_e, GE_e, ANY_e, NONE_e };

    /** DefaultCtor NONE. */
    Rel()
    : _op( NONE_e )
    {}

    /** Ctor from string.
     * Legal values for \a strval_r are: "==", "!=", "<", "<=", ">", ">=",<BR>
     * as well as "EQ", "NE", "LT", "LE", "GT", "GE", "ANY", "NONE"<BR>
     * and "" (empty string resolves to NONE).
     *
     * Lower case names are accepted as well.
     *
     * \throw PARSE if \a strval_r is not legal.
     * \todo refine exceptions and check throw.
    */
    explicit
    Rel( const std::string & strval_r );

    /** String representation of relational operator.
     * \return "==", "!=", "<", "<=", ">", ">=", "ANY" or "NONE"
    */
    const std::string & asString() const;

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

  private:
    /** Ctor to initialize the relational operator contants. */
    Rel( for_use_in_switch op_r )
    : _op( op_r )
    {}
    /** The operator. */
    for_use_in_switch _op;

    friend bool operator==( const Rel & lhs, const Rel & rhs )
    { return lhs._op == rhs._op; }

    friend bool operator!=( const Rel & lhs, const Rel & rhs )
    { return lhs._op != rhs._op; }

  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Rel Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const Rel & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REL_H
