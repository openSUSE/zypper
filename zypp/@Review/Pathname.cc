/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Pathname.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>

#include <y2util/Pathname.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : DirStack
//
//	DESCRIPTION :
//
class DirStack {

  struct Dir {

    Dir *  up;
    Dir *  dn;
    string name;

    Dir( const string & n = "" ) {
      name = n;
      up = dn = 0;
    }

    ~Dir() {
      if ( up )
	up->dn = dn;
      if ( dn )
	dn->up = up;
    }
  };

  Dir *  top;
  Dir *  bot;

  void Pop() {
    if ( !top )
      return;
    top = top->dn;
    if ( top )
      delete top->up;
    else {
      delete bot;
      bot = 0;
    }
  }

  public:

    DirStack() { top = bot = 0; }
    ~DirStack() {
      while ( bot )
	Pop();
    }

    void Push( const string & n ) {
      if ( n.empty() || n == "." ) { // '.' or '/' only for bot
	if ( bot )
	  return;
      } else if ( n == ".." && top ) {
	if ( top->name == "" )          // "/.."        ==> "/"
	  return;

	if ( top->name != "." && top->name != ".." ) {      // "somedir/.." ==> ""
	  Pop();
	  return;
	}
	// "../.." "./.." stays
      }

      Dir * d = new Dir( n );
      if ( !top )
	top = bot = d;
      else {
	top->up = d;
	d->dn = top;
	d->up = 0;
	top = d;
      }
    }

    string str() {
      if ( !bot )
	return "";
      string ret;
      for ( Dir * d = bot; d; d = d->up ) {
	if ( d != bot )
	  ret += "/";
	ret += d->name;
      }
      if ( ret.empty() )
	return "/";
      return ret;
    }
};

///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Pathname::_assign
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void Pathname::_assign( const string & name_tv )
{
  prfx_i = 0;
  name_t = name_tv;

  if ( name_t.empty() )
    return;

  string   Tprfx;
  DirStack Stack_Ci;

  char *       Buf_aci    = new char[name_tv.length() + 1];
  char *       W_pci      = Buf_aci;
  const char * R_pci      = name_tv.c_str();

  // check for prefix
  if (    name_t.length() >= 2
       && name_t[1] == ':'
       && (    'a' <= name_t[0] && name_t[0] <= 'z'
	    || 'A' <= name_t[0] && name_t[0] <= 'Z' ) ) {
    Tprfx  = name_t.substr( 0, 2 );
    prfx_i = 2;
    R_pci += 2;
  }

  // rel or abs path
  if ( *R_pci == '/' || *R_pci == '\\' ) {
    Stack_Ci.Push( "" );
    ++R_pci;
  } else {
    Stack_Ci.Push( "." );
  }

  do {
    switch ( *R_pci ) {
    case '/':
    case '\\':
    case '\0':
      if ( W_pci != Buf_aci ) {
	*W_pci = '\0';
	W_pci = Buf_aci;
	Stack_Ci.Push( Buf_aci );
      }
      break;

    default:
      *W_pci++ = *R_pci;
      break;
    }
  } while( *R_pci++ );

  delete Buf_aci;
  name_t = Tprfx + Stack_Ci.str();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Pathname::dirname
//	METHOD TYPE : Pathname
//
//	DESCRIPTION :
//
Pathname Pathname::dirname( const Pathname & name_tv )
{
  if ( name_tv.empty() )
    return "";

  Pathname ret_t( name_tv );
  string::size_type idx = ret_t.name_t.find_last_of( '/' );

  if ( idx == string::npos ) {
    ret_t.name_t.erase( ret_t.prfx_i );
    ret_t.name_t += ".";
  } else if ( idx == ret_t.prfx_i ) {
    ret_t.name_t.erase( ret_t.prfx_i );
    ret_t.name_t += "/";
  } else {
    ret_t.name_t.erase( idx );
  }

  return ret_t;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Pathname::basename
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string Pathname::basename( const Pathname & name_tv )
{
  if ( name_tv.empty() )
    return "";

  string ret_t( name_tv.asString() );
  ret_t.erase( 0, name_tv.prfx_i );
  string::size_type idx = ret_t.find_last_of( '/' );
  if ( idx != string::npos ) {
    ret_t.erase( 0, idx+1 );
  }

  return ret_t;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Pathname::cat
//	METHOD TYPE : Pathname
//
//	DESCRIPTION :
//
Pathname Pathname::cat( const Pathname & name_tv, const Pathname & add_tv )
{
  if ( add_tv.empty() )
    return name_tv;
  if ( name_tv.empty() )
    return add_tv;

  string ret_ti( add_tv.asString() );
  ret_ti.replace( 0, add_tv.prfx_i, "/" );

  return name_tv.asString() + ret_ti;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Pathname::Extend
//	METHOD TYPE : Pathname
//
//	DESCRIPTION :
//
Pathname Pathname::extend( const Pathname & l, const string & r )
{
  return l.asString() + r;
}

///////////////////////////////////////////////////////////////////
//
//
//      METHOD NAME : Pathname::equal
//      METHOD TYPE : bool
//
//      DESCRIPTION :
//
bool Pathname::equal( const Pathname & l, const Pathname & r )
{
  return l.asString() == r.asString();
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : inline std::ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const Pathname & obj )
{
  return str << obj.asString();
}
