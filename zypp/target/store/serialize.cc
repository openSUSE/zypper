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
#include "zypp/Source.h"

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
  if ( dep[Dep::PROVIDES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::PROVIDES]), "provides") << std::endl;
  if ( dep[Dep::PREREQUIRES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::PREREQUIRES]), "prerequires") << std::endl;
  if ( dep[Dep::REQUIRES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::REQUIRES]), "requires") << std::endl;
  if ( dep[Dep::CONFLICTS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::CONFLICTS]), "conflicts") << std::endl;
   if ( dep[Dep::OBSOLETES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::OBSOLETES]), "obsoletes") << std::endl;
  // why the YUM tag is freshen without s????
   if ( dep[Dep::FRESHENS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::FRESHENS]), "freshen") << std::endl;
   if ( dep[Dep::SUGGESTS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::SUGGESTS]), "suggests") << std::endl;
   if ( dep[Dep::RECOMMENDS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::RECOMMENDS]), "recommends") << std::endl;
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
  out << "<message>" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << "  <text>" << obj->text() << "</text>" << std::endl;
  out << "</message>" << std::endl;
  return out.str();
}

/*

NOT NEEDED FOR NOW, WE JUST LOSE THE TRANSLATION AT STORAGE TIME

static std::string localizedXMLTags( const TranslatedText &text, const std::string &tagname)
{
  stringstream out;
  std::set<Locale> locales = text.locales();
  for(std::set<Locale>::const_iterator it = locales.begin(); it != locales.end(); ++it)
  {
    Locale lcl = *it;
    out << "    <" << tagname << " lang='" << lcl.name() << "'>" << text.text(lcl) << "</" << tagname <<">" << std::endl;
  }
  return out.str();
}
*/

template<> // or constPtr?
std::string toXML( const Selection::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  out << "<pattern" << std::endl;
  out << "  xmlns=\"http://novell.com/package/metadata/suse/pattern\"" << std::endl;
  out << "  xmlns:yum=\"http://linux.duke.edu/metadata/common\"" << std::endl;
  out << "  xmlns:rpm=\"http://linux.duke.edu/metadata/rpm\"" << std::endl;
  out << "  xmlns:suse=\"http://novell.com/package/metadata/suse/common\">" << std::endl;
  out << "  <name>" << obj->name() << "</name>" << std::endl;
  out << "  <summary>" << obj->summary() << "</summary>" << std::endl;
  //out << "  <summary lang='en.US'>foobar</summary>" << std::endl;
  //out << "  <default>" << (obj->isDefault() ? "true" : "false" ) << "</default>" << std::endl;
  out << "  <uservisible>" << (obj->visible() ? "true" : "false" ) << "</uservisible>" << std::endl;
  out << "  <category>" << obj->category() << "</category>" << std::endl;
  out << toXML(obj->deps()) << std::endl;
  out << "</pattern>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Pattern::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  out << "<pattern" << std::endl;
  out << "  xmlns=\"http://novell.com/package/metadata/suse/pattern\"" << std::endl;
  out << "  xmlns:yum=\"http://linux.duke.edu/metadata/common\"" << std::endl;
  out << "  xmlns:rpm=\"http://linux.duke.edu/metadata/rpm\"" << std::endl;
  out << "  xmlns:suse=\"http://novell.com/package/metadata/suse/common\">" << std::endl;
  out << "  <name>" << obj->name() << "</name>" << std::endl;
  out << "  <summary>" << obj->summary() << "</summary>" << std::endl;
  //out << "  <summary lang='en.US'>foobar</summary>" << std::endl;
  out << "  <default>" << (obj->isDefault() ? "true" : "false" ) << "</default>" << std::endl;
  out << "  <uservisible>" << (obj->userVisible() ? "true" : "false" ) << "</uservisible>" << std::endl;
  out << "  <category>" << obj->category() << "</category>" << std::endl;
  out << "  <icon>" << obj->icon() << "</icon>" << std::endl;
  out << "  <script>" << obj->script() << "</script>" << std::endl;
  //out << "  <description lang='cs.CZ'>This is my pattern, it is soooooooo coool!</description>" << std::endl;
  //out << "  <description lang='en.US'>Duh</description>" << std::endl;
  out << toXML(obj->deps()) << std::endl;
  out << "</pattern>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const Product::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  out << "<product";
  out << "    xmlns=\"http://novell.com/package/metadata/suse/product\"" << std::endl;
  out << "    xmlns:product=\"http://novell.com/package/metadata/suse/product\"" << std::endl;
  out << "    xmlns:yum=\"http://linux.duke.edu/metadata/common\" " << std::endl;
  out << "    xmlns:rpm=\"http://linux.duke.edu/metadata/rpm\" " << std::endl;
  out << "    xmlns:suse=\"http://novell.com/package/metadata/suse/common\"" << std::endl;
  out << "    type=\"" << obj->category() << "\">" << std::endl;
  out << "  <vendor>" << obj->vendor() << "</vendor>" << std::endl;
  out << "  <suse:source>" << obj->source().alias() << "</suse:source>" << std::endl;
  out << toXML(static_cast<Resolvable::constPtr>(obj)) << std::endl;
  #warning "FIXME description and displayname of products"
  out << "  <displayname lang=\"en\">" << obj->displayName() << "</displayname>" << std::endl;
  out << "  <suse:release-notes-url>" << obj->releaseNotesUrl() << "</suse:release-notes-url>" << std::endl;
  out << "  <description></description>" << std::endl;
  out << "</product>" << std::endl;

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
  if ( isKind<Product>(resolvable) )
     out << toXML(asKind<const Product>(resolvable)) << std::endl;
  if ( isKind<Pattern>(resolvable) )
     out << toXML(asKind<const Pattern>(resolvable)) << std::endl;
  if ( isKind<Selection>(resolvable) )
     out << toXML(asKind<const Selection>(resolvable)) << std::endl;
  return out.str();
}

// or constPtr?
std::string resolvableTypeToString( const Resolvable::constPtr &resolvable, bool plural )
{
  return resolvableKindToString(resolvable->kind(), plural);
}

std::string resolvableKindToString( const Resolvable::Kind &kind, bool plural)
{
  if ( kind == ResTraits<zypp::Package>::kind )
     return plural ? "packages" : "package";
  else if ( kind == ResTraits<zypp::Patch>::kind )
     return plural ? "patches" : "patch";
  else if ( kind == ResTraits<zypp::Message>::kind )
     return plural ? "messages" : "message";
  else if ( kind == ResTraits<zypp::Selection>::kind )
     return plural ? "selections" : "selection";
  else if ( kind == ResTraits<zypp::Script>::kind )
     return plural ? "scripts" : "script";
  else if ( kind == ResTraits<zypp::Pattern>::kind )
     return plural ? "patterns" : "pattern";
  else if ( kind == ResTraits<zypp::Product>::kind )
     return plural ? "products" : "product";
  else
     return "unknown";
}

template<> // or constPtr?
std::string toXML( const Patch::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  out << "<patch" << std::endl; 
  out << "    xmlns=\"http://novell.com/package/metadata/suse/patch\"" << std::endl;
  out << "    xmlns:patch=\"http://novell.com/package/metadata/suse/patch\"" << std::endl;
  out << "    xmlns:yum=\"http://linux.duke.edu/metadata/common\"" << std::endl; 
  out << "    xmlns:rpm=\"http://linux.duke.edu/metadata/rpm\"" << std::endl;
  out << "    xmlns:suse=\"http://novell.com/package/metadata/suse/common\"" << std::endl;
  out << "    patchid=\"" << obj->id() << "\"" << std::endl;
  out << "    timestamp=\"" << obj->timestamp().asSeconds() << "\"" << std::endl;
  out << "    engine=\"1.0\">" << std::endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  Patch::AtomList at = obj->atoms();
  out << "  <atoms>" << std::endl;
  for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
  {
    // atoms tag here looks weird but lets follow YUM
    
    // I have a better idea to avoid the cast here (Michaels code in his tmp/)
    Resolvable::Ptr one_atom = *it;
    out << castedToXML(one_atom) << std::endl;
  }
  out << "  </atoms>" << std::endl;
  out << "</patch>" << std::endl;
  return out.str();
}

template<> // or constPtr?
std::string toXML( const PersistentStorage::SourceData &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  out << "<source-cache  xmlns=\"http://novell.com/package/metadata/suse/source-cache\">" << std::endl;
  out << "  <enabled>" << obj.enabled << "</enabled>" << std::endl;
  out << "  <auto-refresh>" << obj.autorefresh << "</auto-refresh>" << std::endl;
  out << "  <product-dir>" << obj.product_dir << "</product-dir>" << std::endl;
  out << "  <type>" << obj.type << "</type>" << std::endl;
   out << "  <url>" << obj.url << "</url>" << std::endl;
   out << "  <alias>" << obj.alias << "</alias>" << std::endl;
  out << "</source-cache>" << std::endl;
  return out.str();
}

/////////////////////////////////////////////////////////////////
} // namespace storage
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
