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
#include "zypp/CapFactory.h"

#include "serialize.h"
#include "xml_escape_parser.hpp"

using namespace std;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

std::string xml_escape( const std::string &text )
{
  iobind::parser::xml_escape_parser parser;
  return parser.escape(text);
}

std::string xml_tag_enclose( const std::string &text, const std::string &tag, bool escape = false )
{
  std::string result;
  result += "<" + tag + ">";
  
  if ( escape)
   result += xml_escape(text);
  else
   result += text;

  result += "</" + tag + ">";
  return result;
}

template<class T>
std::string toXML( const T &obj ); //undefined

template<> // or constPtr?
std::string toXML( const Edition &edition )
{
  stringstream out;
  // sad the yum guys did not acll it edition
  out << "<version ver=\"" << edition.version() << "\" rel=\"" << edition.release() << "\"/>";
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Arch &arch )
{
  stringstream out;
  out << xml_tag_enclose(arch.asString(), "arch");
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Capability &cap )
{
  stringstream out;
  CapFactory factory;
  
  out << "<entry kind=\"" << cap.refers() << "\" >" <<  xml_escape(factory.encode(cap)) << "</entry>";
  return out.str();
}

template<> // or constPtr?
std::string toXML( const CapSet &caps )
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
std::string toXML( const Dependencies &dep )
{
  stringstream out;
  if ( dep.provides.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.provides), "provides") << std::endl;
  if ( dep.prerequires.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.prerequires), "prerequires") << std::endl;
  if ( dep.requires.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.requires), "requires") << std::endl;
  if ( dep.conflicts.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.conflicts), "conflicts") << std::endl;
   if ( dep.obsoletes.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.obsoletes), "obsoletes") << std::endl;
  // why the YUM tag is freshen without s????
   if ( dep.freshens.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.freshens), "freshen") << std::endl;
   if ( dep.suggests.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.suggests), "suggests") << std::endl;
   if ( dep.recommends.size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep.recommends), "recommends") << std::endl;
  return out.str();
  
}

template<> // or constPtr?
std::string toXML( const Resolvable::constPtr &obj )
{
  stringstream out;
  
  out << "  <name>" << obj->name() << "</name>" << std::endl;
  // is this shared? uh
  out << "  " << toXML(obj->edition()) << std::endl;
  out << "  " << toXML(obj->arch()) << std::endl;
  out << "  " << toXML(obj->deps()) << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Package::constPtr &obj )
{
  stringstream out;
  out << "<package>" << std::endl;
  // reuse Resolvable information serialize function
  toXML(static_cast<Resolvable::constPtr>(obj));
  //out << "  <do>" << std::endl;
  //out << "      " << obj->do_script() << std::endl;
  //out << "  </do>" << std::endl;
  out << "</package>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Script::constPtr &obj )
{
  stringstream out;
  out << "<script>" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
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
std::string toXML( const Message::constPtr &obj )
{
  stringstream out;
  out << "<message type=\"" << obj->type() << "\">" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << "  <text>" << obj->text() << "</text>" << std::endl;
  out << "</message>" << std::endl;
  return out.str();
}

// or constPtr?
std::string castedToXML( const Resolvable::constPtr &resolvable )
{
  stringstream out;
  if ( isKind<Package>(resolvable) )
     out << toXML(asKind<const Package>(resolvable)) << std::endl;
  if ( isKind<Patch>(resolvable) )
     out << toXML(asKind<const Patch>(resolvable)) << std::endl;
  if ( isKind<Message>(resolvable) )
     out << toXML(asKind<const Message>(resolvable)) << std::endl;
  if ( isKind<Script>(resolvable) )
     out << toXML(asKind<const Script>(resolvable)) << std::endl;
  return out.str();
}

// or constPtr?
std::string resolvableTypeToString( const Resolvable::constPtr &resolvable, bool plural )
{
  return resolvableKindToString(resolvable->kind(), plural);
}

std::string resolvableKindToString( Resolvable::Kind kind, bool plural)
{
  if ( kind == ResTraits<zypp::Package>::kind )
     return plural ? "packages" : "package";
  if ( kind == ResTraits<zypp::Patch>::kind )
     return plural ? "patches" : "patch";
  if ( kind == ResTraits<zypp::Message>::kind )
     return plural ? "messages" : "message";
  if ( kind == ResTraits<zypp::Script>::kind )
     return plural ? "scripts" : "script";
  return "";
}

template<> // or constPtr?
std::string toXML( const Patch::constPtr &obj )
{
  stringstream out;
  out << "<patch>" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  Patch::AtomList at = obj->atoms();
  for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
  {
    // atoms tag here looks weird but lets follow YUM
    out << "  <atoms>" << std::endl;
    // I have a better idea to avoid the cast here (Michaels code in his tmp/)
    Resolvable::Ptr one_atom = *it;
    out << castedToXML(one_atom) << std::endl;
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
