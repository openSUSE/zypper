/*---------------------------------------------------------------------\
 |                          ____ _   __ __ ___                          |
 |                         |__  / \ / / . \ . \                         |
 |                           / / \ V /|  _/  _/                         |
 |                          / /__ | | | | | |                           |
 |                         /_____||_| |_| |_|                           |
 |                                                                      |
 \---------------------------------------------------------------------*/
/** \file	zypp/Pathname.cc
 *
*/
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"

using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::_assign
    //	METHOD TYPE : void
    //
    void Pathname::_assign( const string & name_r )
    {
      _name.clear();
      if ( name_r.empty() )
        return;
      _name.reserve( name_r.size() );

      // Collect up to "/.."
      enum Pending {
	P_none	= 0,	// ""
	P_slash	= 1,	// "/"
	P_dot1	= 2,	// "/."
	P_dot2	= 3	// "/.."
      } pending = P_none;

      // Assert relative path starting with "./"
      // We rely on this below!
      if ( name_r[0] != '/' )
      {
	_name += '.';
	pending = P_slash;
      }

      // Lambda handling the "/.." case:
      // []      + "/.."  ==> []
      // [.]     + "/.."  ==> [./..]
      // [foo]   is always [./foo] due to init above
      // [*/..]  + "/.."  ==> [*/../..]
      // [*/foo] + "/.."  ==> [*]
      auto goParent_f =  [&](){
	if ( _name.empty() )
	  /*NOOP*/;
	else if ( _name.size() == 1 ) // content is '.'
	  _name += "/..";
	else
	{
	  std::string::size_type pos = _name.rfind( "/" );
	  if ( pos == _name.size() - 3 && _name[pos+1] == '.' && _name[pos+2] == '.' )
	    _name += "/..";
	  else
	    _name.erase( pos );
	}
      };

      for ( char ch : name_r )
      {
	switch ( ch )
	{
	  case '/':
	    switch ( pending )
	    {
	      case P_none:	pending = P_slash; break;
	      case P_slash:	break;
	      case P_dot1:	pending = P_slash; break;
	      case P_dot2:	goParent_f(); pending = P_slash; break;
	    }
	    break;

	  case '.':
	    switch ( pending )
	    {
	      case P_none:	_name += '.'; break;
	      case P_slash:	pending = P_dot1; break;
	      case P_dot1:	pending = P_dot2; break;
	      case P_dot2:	_name += "/..."; pending = P_none; break;
	    }
	    break;

	  default:
	    switch ( pending )
	    {
	      case P_none:	break;
	      case P_slash:	_name += '/';	 pending = P_none; break;
	      case P_dot1:	_name += "/.";	 pending = P_none; break;
	      case P_dot2:	_name += "/.."; pending = P_none; break;
	    }
	    _name += ch;
	    break;
	}
      }

      switch ( pending )
      {
	case P_none:	break;
	case P_slash:	if ( _name.empty() ) _name = "/"; break;
	case P_dot1:	if ( _name.empty() ) _name = "/"; break;
	case P_dot2:	goParent_f(); if ( _name.empty() ) _name = "/"; break;
      }
      return;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::dirname
    //	METHOD TYPE : Pathname
    //
    Pathname Pathname::dirname( const Pathname & name_r )
    {
      if ( name_r.empty() )
        return Pathname();

      Pathname ret_t( name_r );
      string::size_type idx = ret_t._name.find_last_of( '/' );

      if ( idx == string::npos ) {
        ret_t._name = ".";
      } else if ( idx == 0 ) {
        ret_t._name = "/";
      } else {
        ret_t._name.erase( idx );
      }

      return ret_t;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::basename
    //	METHOD TYPE : string
    //
    string Pathname::basename( const Pathname & name_r )
    {
      if ( name_r.empty() )
        return string();

      string ret_t( name_r.asString() );
      string::size_type idx = ret_t.find_last_of( '/' );
      if ( idx != string::npos && ( idx != 0 || ret_t.size() != 1 ) ) {
        ret_t.erase( 0, idx+1 );
      }

      return ret_t;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::asUrl
    //	METHOD TYPE : Url
    //
    Url Pathname::asUrl( const std::string & scheme_r ) const
    {
      Url ret;
      ret.setPathName( asString() );
      ret.setScheme( scheme_r );
      return ret;
    }

    Url Pathname::asUrl() const
    { return asUrl( "dir" ); }

    Url Pathname::asDirUrl() const
    { return asUrl( "dir" ); }

    Url Pathname::asFileUrl() const
    { return asUrl( "file" ); }


    std::string Pathname::showRoot( const Pathname & root_r, const Pathname & path_r )
    {
      return str::Str() << "(" << root_r << ")" << path_r;
    }

    std::string Pathname::showRootIf( const Pathname & root_r, const Pathname & path_r )
    {
      if ( root_r.empty() || root_r == "/" )
        return path_r.asString();
      return showRoot( root_r, path_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::extension
    //	METHOD TYPE : string
    //
    string Pathname::extension( const Pathname & name_r )
    {
      if ( name_r.empty() )
        return string();

      string base( basename( name_r ) );
      string::size_type pos = base.rfind( '.' );
      switch ( pos )
      {
	case 0:
	  if ( base.size() == 1 )			// .
	    return string();
	  break;
	case 1:
	  if ( base.size() == 2 && base[0] == '.' )	// ..
	    return string();
	  break;
	case string::npos:
	  return string();
	  break;
      }
      return base.substr( pos );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::assertprefix
    //	METHOD TYPE : Pathname
    //
    Pathname Pathname::assertprefix( const Pathname & root_r, const Pathname & path_r )
    {
      if ( root_r.empty()
           || path_r == root_r
           || str::hasPrefix( path_r.asString(), root_r.asString() ) )
        return path_r;
      return root_r / path_r;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::cat
    //	METHOD TYPE : Pathname
    //
    Pathname Pathname::cat( const Pathname & name_r, const Pathname & add_tv )
    {
      if ( add_tv.empty() )
        return name_r;
      if ( name_r.empty() )
        return add_tv;

      string ret_ti( name_r._name );
      if( add_tv._name[0] != '/' )
	ret_ti += '/';
      return ret_ti + add_tv._name;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Pathname::Extend
    //	METHOD TYPE : Pathname
    //
    Pathname Pathname::extend( const Pathname & l, const string & r )
    {
      return l.asString() + r;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
