/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/LocaleGuard.h
 */
#ifndef ZYPP_BASE_LOCALEGUARD_H
#define ZYPP_BASE_LOCALEGUARD_H

#include <locale.h>
#include <string>

#include "zypp/base/Easy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class LocaleGuard
  /// \brief Temorarily change a locale category value
  /// \ingroup g_RAII
  ///////////////////////////////////////////////////////////////////
  class LocaleGuard
  {
    NON_COPYABLE(LocaleGuard);
    NON_MOVABLE(LocaleGuard);

  public:
    /** Ctor saving the current locale category value. */
    LocaleGuard( int category_r, const std::string & value_r = "C" )
    : _category( -1 )
    {
      const char * ovalue = ::setlocale( category_r, nullptr );
      if ( ovalue && ovalue != value_r )
      {
	_category = category_r;
	_value    = ovalue;
	::setlocale( _category, value_r.c_str() );
      }
    }

    /** Dtor asserts the saved locale category value is restored. */
    ~LocaleGuard()
    { restore(); }

    /** immediately restore the saved locale category value. */
    void restore()
    {
      if ( _category != -1 )
      {
	::setlocale( _category, _value.c_str() );
	_category = -1;
      }
    }

  private:
    int         _category;	///< saved category or -1 if no restore needed
    std::string _value;		///< saved category value
  };
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_LOCALEGUARD_H
