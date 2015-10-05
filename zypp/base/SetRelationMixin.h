/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SetRelationMixin.h
 */
#ifndef ZYPP_BASE_SETRELATIONMIXIN_H
#define ZYPP_BASE_SETRELATIONMIXIN_H

#include <iosfwd>
#include <string>

#include "zypp/base/Easy.h"
#include "zypp/base/EnumClass.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class ESetCompareDef
  /// \brief Result of set comparison (use like 'enum class \ref SetCompare')
  /// This is the type a \c compare function should return.
  ///////////////////////////////////////////////////////////////////
  struct ESetCompareDef {
    enum Enum {
      uncomparable	= 0,		///< "{?}"
      equal		= (1<<0),	///< "{=}"
      properSubset	= (1<<1),	///< "{<}"
      properSuperset	= (1<<2),	///< "{>}"
      disjoint		= (1<<3),	///< "{ }"
    };
    /** String representantion */
    static const std::string & asString( Enum val_r );
  };
  /** \relates ESetCompareDef typedef 'enum class SetCompare' */
  typedef base::EnumClass<ESetCompareDef> SetCompare;

  /** \relates SetCompare Stream output */
  inline std::ostream & operator<<( std::ostream & str, const SetCompare::Enum & obj )
  { return str << SetCompare::asString( obj ); }
  /** \overload */
  inline std::ostream & operator<<( std::ostream & str, const SetCompare & obj )
  { return str << obj.asEnum(); }

  ///////////////////////////////////////////////////////////////////
  /// \class ESetRelationDef
  /// \brief Set Relation based on \ref SetCompare (use like 'enum class \ref SetRelation')
  /// Comparison (\c== \c!=) between \ref SetRelation  and \ref SetCompare
  /// is defined to let \c SetRelation::subset match \c SetCompare::equal
  /// as well as \c SetCompare::properSubset. Accordingly \c SetRelation::subset
  /// matches \c SetCompare::equal as well as \c SetCompare::properSuperset.
  ///////////////////////////////////////////////////////////////////
  struct ESetRelationDef {
    enum Enum {
      uncomparable	= SetCompare::uncomparable,	///< "{??}"
      equal		= SetCompare::equal,		///< "{==}"
      properSubset	= SetCompare::properSubset,	///< "{<<}"
      properSuperset	= SetCompare::properSuperset,	///< "{>>}"
      disjoint		= SetCompare::disjoint,		///< "{  }"
      subset		= properSubset|equal,		///< "{<=}"
      superset		= properSuperset|equal,		///< "{>=}"
    };
    /** String representantion */
    static const std::string & asString( Enum val_r );
  };
  /** \relates ESetRelationDef typedef 'enum class SetRelation' */
  typedef base::EnumClass<ESetRelationDef> SetRelation;

  /** \relates SetRelation Stream output */
  inline std::ostream & operator<<( std::ostream & str, const SetRelation::Enum & obj )
  { return str << SetRelation::asString( obj ); }
  /** \overload */
  inline std::ostream & operator<<( std::ostream & str, const SetRelation & obj )
  { return str << obj.asEnum(); }

  /** \relates SetRelation \relates SetCompare Matching \ref SetCompare and \ref SetRelation */
  inline bool operator==( const SetRelation::Enum & lhs, const SetCompare::Enum & rhs )
  { return( lhs&rhs || !(lhs|rhs) ); }
  /** \overload */
  inline bool operator==( const SetRelation::Enum & lhs, const SetCompare & rhs )
  { return( lhs == rhs.asEnum() ); }
  /** \overload */
  inline bool operator==( const SetRelation & lhs, const SetCompare::Enum & rhs )
  { return( lhs.asEnum() == rhs ); }
  /** \overload */
  inline bool operator==( const SetRelation & lhs, const SetCompare & rhs )
  { return( lhs.asEnum() == rhs.asEnum() ); }
  /** \overload */
  inline bool operator==( const SetCompare::Enum & lhs, const SetRelation::Enum & rhs )
  { return( rhs == lhs ); }
  /** \overload */
  inline bool operator==( const SetCompare::Enum & lhs, const SetRelation & rhs )
  { return( rhs == lhs ); }
  /** \overload */
  inline bool operator==( const SetCompare & lhs, const SetRelation::Enum & rhs )
  { return( rhs == lhs ); }
  /** \overload */
  inline bool operator==( const SetCompare & lhs, const SetRelation & rhs )
  { return( rhs == lhs ); }

  /** \relates SetRelation \relates SetCompare Matching \ref SetCompare and \ref SetRelation */
  inline bool operator!=( const SetRelation::Enum & lhs, const SetCompare::Enum & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetRelation::Enum & lhs, const SetCompare & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetRelation & lhs, const SetCompare::Enum & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetRelation & lhs, const SetCompare & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetCompare::Enum & lhs, const SetRelation::Enum & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetCompare::Enum & lhs, const SetRelation & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetCompare & lhs, const SetRelation::Enum & rhs )
  { return !( lhs == rhs ); }
  /** \overload */
  inline bool operator!=( const SetCompare & lhs, const SetRelation & rhs )
  { return !( lhs == rhs ); }

  ///////////////////////////////////////////////////////////////////
  namespace base
  {
    ///////////////////////////////////////////////////////////////////
    /// \class SetRelationMixin
    /// \brief Provide set relation methods based on Derived::setRelationMixinCompare
    /// A class using this mixin must provide:
    /// \code
    ///   SetCompare setRelationMixinCompare( const Derived & rhs ) const;
    /// \endcode
    /// \see \ref SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN
    /// \ingroup g_CRTP
    ///////////////////////////////////////////////////////////////////
    template <class Derived>
    class SetRelationMixin
    {
    public:
      /** Compare sets */
      SetCompare compare( const Derived & trg ) const
      { return derived().setRelationMixinCompare( trg ); }
      /** \overload */
      SetCompare compare( const SetRelationMixin<Derived> & trg ) const
      { return compare( trg.derived() ); }

      /** Compare sets and match against \ref SetCompare */
      bool compare( const Derived & trg, SetCompare cmp ) const
      { return compare( trg ) == cmp; }
      /** \overload */
      bool compare( const SetRelationMixin<Derived> & trg, SetCompare cmp ) const
      { return compare( trg ) == cmp; }

      /** Compare sets and match against \ref SetRelation */
      bool compare( const Derived & trg, SetRelation rel ) const
      { return compare( trg ) == rel; }
      /** \overload */
      bool compare( const SetRelationMixin<Derived> & trg, SetRelation rel ) const
      { return compare( trg ) == rel; }

    protected:
      SetRelationMixin() {}
      DEFAULT_COPYABLE( SetRelationMixin );
      DEFAULT_MOVABLE( SetRelationMixin );
      ~SetRelationMixin() {}

    private:
      /** Access to sublass Derived*/
      const Derived & derived() const
      { return *static_cast<const Derived*>( this ); }
    };

    /** \relates SetRelationMixin Compare sets */
    template <class Derived>
    inline SetCompare compare( const SetRelationMixin<Derived> & src, const SetRelationMixin<Derived> & trg )
    { return src.compare( trg ); }

    /** \relates SetRelationMixin Compare sets and match against \ref SetCompare */
    template <class Derived>
    inline bool compare( const SetRelationMixin<Derived> & src, const SetRelationMixin<Derived> & trg, SetCompare cmp )
    { return src.compare( trg, cmp ); }

    /** \relates SetRelationMixin Compare sets and match against \ref SetRelation */
    template <class Derived>
    inline bool compare( const SetRelationMixin<Derived> & src, const SetRelationMixin<Derived> & trg, SetRelation rel )
    { return src.compare( trg, rel ); }

    /** \relates SetRelationMixin Equal */
    template <class Derived>
    inline bool operator==( const SetRelationMixin<Derived> & src, const SetRelationMixin<Derived> & trg )
    { return src.compare( trg, SetRelation::equal ); }

    /** \relates SetRelationMixin Unequal */
    template <class Derived>
    inline bool operator!=( const SetRelationMixin<Derived> & src, const SetRelationMixin<Derived> & trg )
    { return !( src == trg ); }

    /** \relates SetRelationMixin Define compare between Derived and some other type (e.g. std::string)
     * \code
     *   class Foo : public base::SetRelationMixin<Foo> {...};
     *   SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( Foo, const char * );
     *   SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN( Foo, const std::string & );
     * \endcode
     */
#define SETRELATIONMIXIN_DEFINE_COMPARE_BETWEEN(DERIVED_TYPE,OTHER_TYPE)	\
  inline SetCompare compare( const base::SetRelationMixin<DERIVED_TYPE> & src, OTHER_TYPE trg )	\
  { return src.compare( DERIVED_TYPE(trg) ); }	\
  inline SetCompare compare( OTHER_TYPE src, const base::SetRelationMixin<DERIVED_TYPE> & trg )	\
  { return DERIVED_TYPE(src).compare( trg ); }	\
	\
  inline bool compare( const base::SetRelationMixin<DERIVED_TYPE> & src, OTHER_TYPE trg, SetCompare cmp )	\
  { return src.compare( DERIVED_TYPE(trg), cmp ); }	\
  inline bool compare( OTHER_TYPE src, const base::SetRelationMixin<DERIVED_TYPE> & trg, SetCompare cmp )	\
  { return DERIVED_TYPE(src).compare( trg, cmp ); }	\
	\
  inline bool compare( const base::SetRelationMixin<DERIVED_TYPE> & src, OTHER_TYPE trg, SetRelation rel )	\
  { return src.compare( DERIVED_TYPE(trg), rel ); }	\
  inline bool compare( OTHER_TYPE src, const base::SetRelationMixin<DERIVED_TYPE> & trg, SetRelation rel )	\
  { return DERIVED_TYPE(src).compare( trg, rel ); }	\
	\
  inline bool operator==( const base::SetRelationMixin<DERIVED_TYPE> & src, OTHER_TYPE trg )	\
  { return src.compare( DERIVED_TYPE(trg), SetRelation::equal ); }	\
  inline bool operator==( OTHER_TYPE src, const base::SetRelationMixin<DERIVED_TYPE> & trg )	\
  { return DERIVED_TYPE(src).compare( trg, SetRelation::equal ); }	\
	\
  inline bool operator!=( const base::SetRelationMixin<DERIVED_TYPE> & src, OTHER_TYPE trg )	\
  { return !( src == trg ); }	\
  inline bool operator!=( OTHER_TYPE src, const base::SetRelationMixin<DERIVED_TYPE> & trg )	\
  { return !( src == trg ); }

  } // namespace base
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_SETRELATIONMIXIN_H
