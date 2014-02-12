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
    /// struct _ColorDef { enum Enum { R, G ,B }; }
    /// typedef EnumClass<_ColorDef> Color;
    /// \endcode
    ///////////////////////////////////////////////////////////////////
    template<typename _EnumDef>
    class EnumClass : public _EnumDef
    {
    public:
      typedef typename _EnumDef::Enum Enum;	///< The underlying enum type

      EnumClass( Enum val_r ) : _val( val_r ) {}

      /** For use in switch
       * \code
       * struct _ColorDef { enum Enum { R, G ,B }; }
       * typedef EnumClass<_ColorDef> Color;
       *
       * Color a;
       * switch ( a.inSwitch() )
       * \endcode
       */
      Enum inSwitch() const { return _val; }

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
