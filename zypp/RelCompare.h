/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/RelCompare.h
 *
*/
#ifndef ZYPP_RELCOMPARE_H
#define ZYPP_RELCOMPARE_H

#include <functional>

#include "zypp/Rel.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** \defgroup RelCompare Comparison using relational operator zypp::Rel.
   *
   * . Take a class like zypp::Edition. Editions are comaprable.
   * You can compare them lexicographical, or according to their
   * version and release values, or match them (i.e. taking empty
   * version or release values as wildcard).
   *
   * No matter which way is appropriate within a certain context.
   * You need functions to compare, and may want to use classes like
   * zypp::Range, based on the desired comparison.
   *
   * All the class has to do, is providing a general comparison
   * method (preferably static)
   * \code
   *     // Compare two elements returning -1, 0, 1
   *     //  if the elemants compare <,==,>.
   *     static int compare( const Tp & lhs, const Tp & rhs );
   * \endcode
   *
   * <tt>Compare\<Tp\></tt> provides a functor wrapping \c compare.
   * In case the general comparison method is named differently, the
   * class, or you, have to provide an approriate functor.
   *
   * <tt>compareByRel</tt> then compares two elements using a certain
   * operator and general comparison method.
   * \code
   * compareByRel( Rel::EQ, lhs, rhs, Edition::compare );
   * compareByRel( Rel::EQ, lhs, rhs, Edition::match );
   * compareByRel( Rel::EQ, lhs, rhs ); // defaults to Compare\<Edition\>
   *                                    // thus Edition::compare
   * \endcode
   *
   * Furthermore a bunch of functors using a certain opertator is
   * defined. All templated by type and general comparison
   * method (defaults to Compare\<Tp\>).
   * \code
   * // Editions sets use lexicographical order per default:
   * std::set<Edition>
   *
   * // An Edition set using Edition::compare as order:
   * std::set<Edition,CompareByLT<Edition> >;
   *
   * // Edition::match is not transitive, thus not an appropriate
   * // order relation for std::set or std::map.
   * \endcode
   *
   * Classes like zypp:Range are templated by  by type and general
   * comparison method as well. Thus you may use Edition ranges based
   * on Edition::Compare, as well as ranges based on Edition::Match
   * (Edition provides these two functors).
   *
   * Again: Everything a class has to provide is the general
   * comparison method. Comparison functors and ranges are then
   * immediately available.
  */
  //@{

  /** General compare functor returning <tt>-1, 0, 1</tt>.
   * Expects Tp::compare to be a static comaprison method
   * returning <tt>-1, 0, 1</tt> if the elements compare
   * <tt>\<,==,\></tt>.
  */
  template<class Tp>
    struct Compare : public std::binary_function<Tp,Tp,int>
    {
      int operator()( const Tp & lhs, const Tp & rhs ) const
      { return Tp::compare( lhs, rhs ); }
    };

  ///////////////////////////////////////////////////////////////////

  /** Comparison of two elements using relational operator \a op.
   * Expects \a TCompare to be a binary operator returning
   * <tt>-1, 0, 1</tt> if the elemants compare <tt>\<,==,\></tt>.
   * \code
   *     // Signature of compare function or functor:
   *     int compare( const Tp & lhs, const Tp & rhs );
   * \endcode
   * \li If \a op is Rel::ANY, the expression is always \c true.
   * \li If \a op is Rel::NONE, the expression is always \c false.
   * \li Otherwise the expression is evaluated using \a compare.
   *
   * \ingroup RelCompare
  */
  template<class Tp, class TCompare>
    inline bool compareByRel( Rel op, const Tp & lhs, const Tp & rhs, TCompare compare )
    {
      switch ( op.inSwitch() )
      {
      case Rel::EQ_e:
        return compare( lhs, rhs ) == 0;
        break;
      case Rel::NE_e:
        return compare( lhs, rhs ) != 0;
        break;
      case Rel::LT_e:
        return compare( lhs, rhs ) < 0;
        break;
      case Rel::LE_e:
        return compare( lhs, rhs ) <= 0;
        break;
      case Rel::GT_e:
        return compare( lhs, rhs ) > 0;
        break;
      case Rel::GE_e:
        return compare( lhs, rhs ) >= 0;
        break;
      case Rel::ANY_e:
        return true;
        break;
      case Rel::NONE_e:
        return false;
        break;
      }
      return false;
    }

  /** \ref compareByRel convenience using Compare<Tp> as general compare
   *  functor.
  */
  template<class Tp>
    inline bool compareByRel( Rel op, const Tp & lhs, const Tp & rhs )
    { return compareByRel( op, lhs, rhs, Compare<Tp>() ); }

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////

  /** Functor to compare two elements by \ref Rel based on
   * a general \a TCompare functor.
   *
   * Expects \a TCompare to be suitable for use in \ref compareByRel.
   * Defaults to Compare\<Tp\>.
  */
  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareBy : public std::binary_function<Tp,Tp,bool>
    {
      CompareBy( Rel op_r )
      : _op( op_r )
      {}

      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( _op, lhs, rhs, TCompare() ); }

      Rel _op;
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByEQ : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::EQ, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByNE : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::NE, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByLT : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::LT, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByLE : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::LE, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByGT : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::GT, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByGE : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::GE, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByANY : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::ANY, lhs, rhs, TCompare() ); }
    };

  template<class Tp, class TCompare = Compare<Tp> >
    struct CompareByNONE : public std::binary_function<Tp,Tp,bool>
    {
      bool operator()( const Tp & lhs, const Tp & rhs ) const
      { return compareByRel( Rel::NONE, lhs, rhs, TCompare() ); }
    };

  ///////////////////////////////////////////////////////////////////

  //@}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RELCOMPARE_H
