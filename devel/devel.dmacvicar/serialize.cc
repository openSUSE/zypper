/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	devel/devel.dmacvicar/SQLiteBackend.cc
*
*/
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "zypp/base/Logger.h"

#include "serialize.h"

using namespace std;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

template<class T>
std::string toXML( T obj ); //undefined

template<> // or constPtr?
std::string toXML( const Edition edition )
{
  stringstream out;
  // sad the yum guys did not acll it edition
  out << "<version ver=\"" << edition.version() << "\" rel=\"" << edition.release() << "\"/>";
  return out.str();
}

template<> // or constPtr?
std::string toXML( Capability cap )
{
  stringstream out;
  out << "<entry kind=\"" << cap.refers() << "\" name=\"" <<  cap.asString() << "\" />";
  return out.str();
}

template<> // or constPtr?
std::string toXML( const CapSet caps )
{
  stringstream out;
  CapSet::iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    out << toXML((*it));
  }
  return out.str(); 
}

template<> // or constPtr?
std::string toXML( const Dependencies dep )
{
  stringstream out;
  out << "    <provides>" << std::endl;
  out << "    " << toXML(dep.provides) << std::endl;
  out << "    </provides>" << std::endl;
  out << "    <prerequires>" << std::endl;
  out << "    " << toXML(dep.prerequires) << std::endl;
  out << "    </prerequires>" << std::endl;
  out << "    <requires>" << std::endl;
  out << "    " << toXML(dep.requires) << std::endl;
  out << "    </requires>" << std::endl;
  out << "    <conflicts>" << std::endl;
  out << "    " << toXML(dep.conflicts) << std::endl;
  out << "    </conflicts>" << std::endl;
  out << "    <obsoletes>" << std::endl;
  out << "    " << toXML(dep.obsoletes) << std::endl;
  out << "    </obsoletes>" << std::endl;  
  out << "    <freshens>" << std::endl;
  out << "    " << toXML(dep.freshens) << std::endl;
  out << "    </freshens>" << std::endl;
  out << "    <suggests>" << std::endl;
  out << "    " << toXML(dep.suggests) << std::endl;
  out << "    </suggest>" << std::endl;
  out << "    <recommends>" << std::endl;
  out << "    " << toXML(dep.recommends) << std::endl;
  out << "    </recommends>" << std::endl;  
  return out.str();
  
}

template<> // or constPtr?
std::string toXML( Resolvable::Ptr obj )
{
  stringstream out;
  
  out << "  <name>" << obj->name() << "</name>" << std::endl;
  // is this shared? uh
  out << "  " << toXML(obj->edition()) << std::endl;
  out << "  " << toXML(obj->deps()) << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( Package::Ptr obj )
{
  stringstream out;
  out << "<package>" << std::endl;
  // reuse Resolvable information serialize function
  toXML(static_cast<Resolvable::Ptr>(obj));
  out << "  <do>" << std::endl;
  //out << "      " << obj->do_script() << std::endl;
  out << "  </do>" << std::endl;
  out << "</package>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( Script::Ptr obj )
{
  stringstream out;
  out << "<script>" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::Ptr>(obj));
  out << "  <do>" << std::endl;
  out << "      " << obj->do_script() << std::endl;
  out << "  </do>" << std::endl;
  if ( obj->undo_available() )
  {
    out << "  <undo>" << std::endl;
    out << "      " << obj->undo_script() << std::endl;
    out << "  </undo>" << std::endl;
  }
  out << "</script>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( Message::Ptr obj )
{
  stringstream out;
  out << "<message type=\"" << obj->type() << "\">" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::Ptr>(obj));
  out << "  <text>" << obj->text() << "</text>" << std::endl;
  out << "</message>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( Patch::Ptr obj )
{
  stringstream out;
  out << "<patch>" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::Ptr>(obj));
  Patch::AtomList at = obj->atoms();
  for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
  {
    // atoms tag here looks weird but lets follow YUM
    out << "  <atoms>" << std::endl;
    // I have a better idea to avoid the cast here (Michaels code in his tmp/)
    Resolvable::Ptr one_atom = *it;
    if ( isKind<Package>(one_atom) )
       out << toXML(asKind<Package>(one_atom)) << std::endl;
    if ( isKind<Patch>(one_atom) )
       out << toXML(asKind<Patch>(one_atom)) << std::endl;
    if ( isKind<Message>(one_atom) )
       out << toXML(asKind<Message>(one_atom)) << std::endl;
    if ( isKind<Script>(one_atom) )
       out << toXML(asKind<Script>(one_atom)) << std::endl;
    out << "  </atoms>" << std::endl;
  }
  out << "</patch>" << std::endl;
  return out.str();
}



/////////////////////////////////////////////////////////////////
} // namespace devel.dmacvicar
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace devel
///////////////////////////////////////////////////////////////////
