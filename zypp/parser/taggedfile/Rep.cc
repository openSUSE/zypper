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

   File:       Rep.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

   Purpose: Base class for reference counted objects and counted pointer templates.

/-*/

#include <iostream>

#include "zypp/parser/taggedfile/Rep.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Rep
//
///////////////////////////////////////////////////////////////////

ostream & Rep::dumpOn( ostream & str ) const {
  return str << repName() << "(<-" << refCount() << ')';
}

ostream & operator<<( ostream & str, const Rep & obj ) {
  return obj.dumpOn( str );
}

ostream & operator<<( ostream & str, const Rep * obj ) {
  if ( ! obj )
    return str << "(NULL)";
  return str << *obj;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : CountedRep
//
///////////////////////////////////////////////////////////////////

unsigned CountedRep::_objectCount = 0;
unsigned CountedRep::_objectIds = 0;

ostream & CountedRep::dumpOn( ostream & str ) const {
  return str << repName() << "[" << objectId() << "(<-" << refCount() << ")]";
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RepPtrBase
//
///////////////////////////////////////////////////////////////////

ostream & operator<<( ostream & str, const RepPtrBase & obj ) {
  return str << obj.refbase();
}
