/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/EnumClass.h
 */
#ifndef ZYPP_BASE_ENUMCLASS_H
#define ZYPP_BASE_ENUMCLASS_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace base
  {
    ///////////////////////////////////////////////////////////////////
    /// \class EnumClass
    /// \brief Type safe enum (workaround SWIG not supporting enum class)
    /// \code
    /// struct TColorDef { enum Enum { R, G ,B }; };
    /// typedef EnumClass<TColorDef> Color;
    /// \endcode
    /// Conversion to from string can be easily added, e.g. like this:
    /// \code
    /// struct TColorDef {
    ///   enum Enum { R, G ,B };
    ///   static Enum fromString( const std::string & val_r );
    ///   static const std::string & asString( Enum val_r );
    /// };
    /// std::ostream & operator<<( std::ostream & str, const TColorDef & obj )
    /// { return str << TColorDef::asString( obj.inSwitch() ); }
    ///
    /// typedef EnumClass<TColorDef> Color;
    /// Color red = Color::fromString("red");
    /// cout << red << endl; // "red"
    /// \endcode
    ///////////////////////////////////////////////////////////////////
    template<typename TEnumDef>
    class EnumClass : public TEnumDef
    {
    public:
      typedef typename TEnumDef::Enum Enum;		///< The underlying enum type
      typedef typename std::underlying_type<Enum>::type Integral;///< The underlying integral type

      EnumClass( Enum val_r ) : _val( val_r ) {}

      /** Underlying enum value for use in switch
       * \code
       * struct TColorDef { enum Enum { R, G ,B }; }
       * typedef EnumClass<TColorDef> Color;
       *
       * Color a;
       * switch ( a.asEnum() )
       * \endcode
       */
      Enum asEnum() const { return _val; }

      /** Underlying integral value (e.g. array index)
       * \code
       * struct TColorDef { enum Enum { R, G ,B }; }
       * typedef EnumClass<TColorDef> Color;
       *
       * Color a;
       * std::string table[] = { "red", "green", "blue" };
       * std::cout << table[a.asIntegral()] << std::endl;
       */
      Integral asIntegral() const { return static_cast<Integral>(_val); }

      friend bool operator==( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val == rhs._val; }
      friend bool operator!=( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val != rhs._val; }
      friend bool operator< ( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val <  rhs._val; }
      friend bool operator<=( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val <= rhs._val; }
      friend bool operator> ( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val >  rhs._val; }
      friend bool operator>=( const EnumClass & lhs, const EnumClass & rhs ) { return lhs._val >= rhs._val; }

    private:
      Enum _val;
    };
  } // namespace base
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_ENUMCLASS_H
