/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#ifndef ZYPP_NG_CORE_STRING_H_INCLUDED
#define ZYPP_NG_CORE_STRING_H_INCLUDED

#include <zypp-core/base/String.h>
#include <boost/utility/string_view.hpp>

namespace zyppng {
using namespace zypp::str;

namespace str {

  template< typename StrType, typename T = std::remove_reference_t<StrType> >
  T trim( StrType&& s, const Trim trim_r )
  {
    T ret( std::forward<StrType>(s) );

    if ( ret.empty() || trim_r == NO_TRIM )
      return ret;

    if ( trim_r & L_TRIM )
    {
      typename T::size_type p = ret.find_first_not_of( " \t\r\n" );
      if ( p == T::npos )
      {
        ret.clear();
        return ret;
      }
      ret.remove_prefix( p );
    }

    if ( trim_r & R_TRIM )
    {
      typename T::size_type p = ret.find_last_not_of( " \t\r\n" );
      if ( p == T::npos )
      {
        ret.clear();
        return ret;
      }
      ret.remove_suffix( ret.size() - ( p+1 ) );
    }

    return ret;
  }

  template<class TOutputIterator>
  void split( const boost::string_view & line_r, TOutputIterator result_r, const boost::string_view & sepchars_r = " \t", const Trim trim_r = NO_TRIM )
  {
    //skip initial sepchars
    std::string_view::size_type tokenEnd = 0, tokenBegin = line_r.find_first_not_of( sepchars_r );

    //if we do not find a character that is not in sepchars there is nothing to split
    if ( tokenBegin == std::string_view::npos )
      return;

    while ( ( tokenEnd = line_r.find_first_of( sepchars_r, tokenBegin ) ) != std::string_view::npos ) {
      auto line = line_r.substr( tokenBegin, tokenEnd-tokenBegin );
      *result_r = trim( line, trim_r );

      //find start of next token
      tokenBegin = line_r.find_first_not_of( sepchars_r, tokenEnd );
      if( tokenBegin == std::string_view::npos )
        break;
    }

    //insert the final element
    if ( tokenBegin != std::string_view::npos && tokenBegin < line_r.size() )
      *result_r = trim( line_r.substr( tokenBegin ), trim_r );
  }
}



}


#endif
