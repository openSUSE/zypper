/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/serialize.cc
*
*/
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <streambuf>

#include <boost/logic/tribool.hpp>

#include "zypp/base/Logger.h"
#include "zypp/Url.h"

#include "zypp/ResObject.h"
#include "zypp/repo/ScriptProvider.h"

#include "serialize.h"

#include "xml_escape_parser.hpp"

using namespace std;
///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

static void copyFileToStream( const Pathname & file_r, std::ostream & out_r )
{
  std::ifstream infile( file_r.c_str() );
  while (infile.good())
  {
    char c = (char)infile.get();
    if (! infile.eof())
      out_r << c;
  }
  infile.close();
}

static string xml_escape( const string &text )
{
  iobind::parser::xml_escape_parser parser;
  return parser.escape(text);
}

static string xml_tag_enclose( const string &text, const string &tag, bool escape = false )
{
  string result;
  result += "<" + tag + ">";

  if ( escape)
   result += xml_escape(text);
  else
   result += text;

  result += "</" + tag + ">";
  return result;
}

static ostream & operator<<( ostream & str, const boost::tribool obj )
{
  if (obj)
    return str << "true";
  else if (!obj)
    return str << "false";
  else
    return str << "indeterminate";
}

/**
 * helper function that builds
 * <tagname lang="code">text</tagname>
 *
 *
 */
static string translatedTextToXML(const TranslatedText &text, const string &tagname)
{
  set<Locale> locales = text.locales();
  //ERR << "locale contains " << locales.size() << " translations" << endl;
  stringstream out;
  for ( set<Locale>::const_iterator it = locales.begin(); it != locales.end(); ++it)
  {
    //ERR << "serializing " << (*it).code() << endl;
    if ( *it == Locale() )
      out << "<" << tagname << ">" << xml_escape(text.text(*it)) << "</" << tagname << ">" << endl;
    else
      out << "<" << tagname << " lang=\"" << (*it).code() << "\">" << xml_escape(text.text(*it)) << "</" << tagname << ">" << endl;
  }
  return out.str();
}

template<class T>
string toXML( const T &obj ); //undefined

template<>
string toXML( const Edition &edition )
{
  stringstream out;
  // sad the yum guys did not acll it edition
  out << "<version ver=\"" << xml_escape(edition.version()) << "\" rel=\"" << xml_escape(edition.release()) << "\" epoch=\"" << edition.epoch() << "\" />";
  return out.str();
}

template<>
string toXML( const Arch &arch )
{
  stringstream out;
  out << xml_tag_enclose(xml_escape(arch.asString()), "arch");
  return out.str();
}

template<>
string toXML( const Capability &cap )
{
#warning FIX WRITING AND READING CAPABILITY (incl old format)
  stringstream out;
  //CapFactory factory;
  //out << "<capability kind=\"" << cap.refers() << "\" >" <<  xml_escape(factory.encode(cap)) << "</capability>" << endl;
  out << "<capability>" <<  cap << "</capability>" << endl; // wrong !

  return out.str();
}

template<>
string toXML( const Capabilities &caps )
{
  stringstream out;
  Capabilities::const_iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    out << toXML((*it));
  }
  return out.str();
}

template<>
string toXML( const CapabilitySet &caps )
{
  stringstream out;
  CapabilitySet::const_iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    out << toXML((*it));
  }
  return out.str();
}

template<>
string toXML( const Dependencies &dep )
{
  stringstream out;
  if ( dep[Dep::PROVIDES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::PROVIDES]), "provides") << endl;
  if ( dep[Dep::PREREQUIRES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::PREREQUIRES]), "prerequires") << endl;
  if ( dep[Dep::CONFLICTS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::CONFLICTS]), "conflicts") << endl;
  if ( dep[Dep::OBSOLETES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::OBSOLETES]), "obsoletes") << endl;
  if ( dep[Dep::FRESHENS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::FRESHENS]), "freshens") << endl;
  if ( dep[Dep::REQUIRES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::REQUIRES]), "requires") << endl;
  if ( dep[Dep::RECOMMENDS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::RECOMMENDS]), "recommends") << endl;
  if ( dep[Dep::ENHANCES].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::ENHANCES]), "enhances") << endl;
  if ( dep[Dep::SUPPLEMENTS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::SUPPLEMENTS]), "supplements") << endl;
  if ( dep[Dep::SUGGESTS].size() > 0 )
    out << "    " << xml_tag_enclose(toXML(dep[Dep::SUGGESTS]), "suggests") << endl;
  return out.str();

}

template<>
string toXML( const Resolvable::constPtr &obj )
{
  stringstream out;

  out << "  <name>" << xml_escape(obj->name()) << "</name>" << endl;
  // is this shared? uh
  out << "  " << toXML(obj->edition()) << endl;
  out << "  " << toXML(obj->arch()) << endl;
  out << "  " << toXML(obj->deps()) << endl;
  return out.str();
}

template<>
string toXML( const ResObject::constPtr &obj )
{
  stringstream out;
#warning FIX WRITING AND READING TRANSLATED TEXTS
  // access implementation
  //detail::ResImplTraits<ResObject::Impl>::constPtr pipp( detail::ImplConnect::resimpl( obj ) );
  //out << translatedTextToXML(pipp->summary(), "summary");
  //out << translatedTextToXML(pipp->description(), "description");

  //out << translatedTextToXML(pipp->insnotify(), "install-notify");
  //out << translatedTextToXML(pipp->delnotify(), "delete-notify");
  //out << translatedTextToXML(pipp->licenseToConfirm(), "license-to-confirm");
  out << "  <vendor>" << xml_escape(obj->vendor()) << "</vendor>" << endl;
  out << "  <size>" << static_cast<ByteCount::SizeType>(obj->size()) << "</size>" << endl;
  out << "  <archive-size>" << static_cast<ByteCount::SizeType>(obj->downloadSize()) << "</archive-size>" << endl;
  out << "  <install-only>" << ( obj->installOnly() ? "true" : "false" ) << "</install-only>" << endl;
  out << "  <build-time>" << obj->buildtime().asSeconds()  << "</build-time>" << endl;
  // we assume we serialize on storeObject, set install time to NOW
  out << "  <install-time>" << Date::now().asSeconds() << "</install-time>" << endl;

  return out.str();
}

template<>
string toXML( const Package::constPtr &obj )
{
  stringstream out;
  out << "<package>" << endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << toXML(static_cast<ResObject::constPtr>(obj));
  out << "</package>" << endl;
  return out.str();
}

template<>
string toXML( const Script::constPtr &obj )
{
  stringstream out;
  out << "<script>" << endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << toXML(static_cast<ResObject::constPtr>(obj));

  repo::RepoMediaAccess access;
  repo::ScriptProvider prov( access );

  out << "  <do>" << endl;
  out << "  <![CDATA[" << endl;
  copyFileToStream( prov.provideDoScript( obj ), out );
  out << "  ]]>" << endl;
  out << "  </do>" << endl;

  if ( obj->undoAvailable() )
  {
    out << "  <undo>" << endl;
    out << "  <![CDATA[" << endl;
    copyFileToStream( prov.provideUndoScript( obj ), out );
    out << "  ]]>" << endl;
    out << "  </undo>" << endl;

  }
  out << "</script>" << endl;
  return out.str();
}

template<>
string toXML( const Message::constPtr &obj )
{
  stringstream out;
  out << "<message>" << endl;
  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << toXML(static_cast<ResObject::constPtr>(obj));
  out << "  <text>" << xml_escape(obj->text().text()) << "</text>" << endl;
  out << "</message>" << endl;
  return out.str();
}

template<>
string toXML( const Language::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<language version=\"" << SERIALIZER_VERSION << "\" xmlns=\"http://www.novell.com/metadata/zypp/xml-store\">" << endl;
  out << toXML(static_cast<Resolvable::constPtr>(obj)) << endl;
  out << toXML(static_cast<ResObject::constPtr>(obj));
  out << "</language>" << endl;
  return out.str();
}


template<>
string toXML( const Selection::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<pattern version=\"" << SERIALIZER_VERSION << "\" xmlns=\"http://www.novell.com/metadata/zypp/xml-store\">" << endl;

  out << toXML(static_cast<Resolvable::constPtr>(obj)) << endl;
  out << toXML(static_cast<ResObject::constPtr>(obj));

  //out << "  <default>" << (obj->isDefault() ? "true" : "false" ) << "</default>" << endl;
  out << "  <uservisible>" << (obj->visible() ? "true" : "false" ) << "</uservisible>" << endl;
  out << "  <category>" << xml_escape(obj->category()) << "</category>" << endl;
  out << "  <icon></icon>" << endl;
  out << "</pattern>" << endl;
  return out.str();
}

template<>
string toXML( const Atom::constPtr &obj )
{
  stringstream out;
  out << "<atom>" << endl;
  out << toXML(static_cast<Resolvable::constPtr>(obj)) << endl;
  out << toXML(static_cast<ResObject::constPtr>(obj));
  out << "</atom>" << endl;
  return out.str();
}

template<>
string toXML( const Pattern::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<pattern version=\"" << SERIALIZER_VERSION << "\" xmlns=\"http://www.novell.com/metadata/zypp/xml-store\">" << endl;

  out << toXML(static_cast<Resolvable::constPtr>(obj)) << endl;
  out << toXML(static_cast<ResObject::constPtr>(obj));

  out << "  <default>" << (obj->isDefault() ? "true" : "false" ) << "</default>" << endl;
  out << "  <uservisible>" << (obj->userVisible() ? "true" : "false" ) << "</uservisible>" << endl;
  out << "  <category>" << xml_escape(obj->category()) << "</category>" << endl;
  out << "  <icon>" << xml_escape(obj->icon().asString()) << "</icon>" << endl;
  out << "  <script>" << xml_escape(obj->script().asString()) << "</script>" << endl;
  out << "</pattern>" << endl;
  return out.str();
}

template<>
string toXML( const Product::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<product version=\"" << SERIALIZER_VERSION << "\" xmlns=\"http://www.novell.com/metadata/zypp/xml-store\" type=\"" << xml_escape(obj->type()) << "\">" << endl;
  out << toXML(static_cast<Resolvable::constPtr>(obj)) << endl;
#warning "FIXME description and displayname of products"

  out << toXML(static_cast<ResObject::constPtr>(obj));

#warning FIX WRITING AND READING TRANSLATED TEXTS
  // access implementation
//   detail::ResImplTraits<Product::Impl>::constPtr pipp( detail::ImplConnect::resimpl( obj ) );
//   out << translatedTextToXML(pipp->shortName(), "shortname");

  out << "  <distribution-name>" << xml_escape(obj->distributionName()) << "</distribution-name>" << endl;
  out << "  <distribution-edition>" << xml_escape(obj->distributionEdition().asString()) << "</distribution-edition>" << endl;
  out << "  <source>" << xml_escape(obj->repository().info().alias()) << "</source>" << endl;
  out << "  <release-notes-url>" << xml_escape(obj->releaseNotesUrl().asString()) << "</release-notes-url>" << endl;

  out << "  <update-urls>" << endl;
  list<Url> updateUrls = obj->updateUrls();
  for ( list<Url>::const_iterator it = updateUrls.begin(); it != updateUrls.end(); ++it)
  {
    out << "    <update-url>" << xml_escape(it->asString()) << "</update-url>" << endl;
  }
  out << "  </update-urls>" << endl;

  out << "  <extra-urls>" << endl;
  list<Url> extraUrls = obj->extraUrls();
  for ( list<Url>::const_iterator it = extraUrls.begin(); it != extraUrls.end(); ++it)
  {
    out << "    <extra-url>" << xml_escape(it->asString()) << "</extra-url>" << endl;
  }
  out << "  </extra-urls>" << endl;

  out << "  <optional-urls>" << endl;
  list<Url> optionalUrls = obj->optionalUrls();
  for ( list<Url>::const_iterator it = optionalUrls.begin(); it != optionalUrls.end(); ++it)
  {
    out << "    <optional-url>" << xml_escape(it->asString()) << "</optional-url>" << endl;
  }
  out << "  </optional-urls>" << endl;

  out << "  <product-flags>" << endl;
  list<string> flags = obj->flags();
  for ( list<string>::const_iterator it = flags.begin(); it != flags.end(); ++it)
  {
    out << "    <product-flag>" << xml_escape(*it) << "</product-flag>" << endl;
  }
  out << "  </product-flags>" << endl;

  out << "</product>" << endl;

  return out.str();
}


string castedToXML( const Resolvable::constPtr &resolvable )
{
  stringstream out;
  if ( isKind<Package>(resolvable) )
     out << toXML(asKind<const Package>(resolvable)) << endl;
  if ( isKind<Patch>(resolvable) )
     out << toXML(asKind<const Patch>(resolvable)) << endl;
  if ( isKind<Message>(resolvable) )
     out << toXML(asKind<const Message>(resolvable)) << endl;
  if ( isKind<Script>(resolvable) )
     out << toXML(asKind<const Script>(resolvable)) << endl;
  if ( isKind<Atom>(resolvable) )
     out << toXML(asKind<const Atom>(resolvable)) << endl;
  if ( isKind<Product>(resolvable) )
     out << toXML(asKind<const Product>(resolvable)) << endl;
  if ( isKind<Pattern>(resolvable) )
     out << toXML(asKind<const Pattern>(resolvable)) << endl;
  if ( isKind<Selection>(resolvable) )
     out << toXML(asKind<const Selection>(resolvable)) << endl;
  if ( isKind<Language>(resolvable) )
    out << toXML(asKind<const Language>(resolvable)) << endl;
  return out.str();
}

string resolvableTypeToString( const Resolvable::constPtr &resolvable, bool plural )
{
  return resolvableKindToString(resolvable->kind(), plural);
}

string resolvableKindToString( const Resolvable::Kind &kind, bool plural)
{
  string k = kind.asString();
  if (k.substr(k.size() - 2, 2) == "ch")
    return k + (plural?"es":"");
  else
    return k + (plural?"s":"");
}

template<>
string toXML( const Patch::constPtr &obj )
{
  stringstream out;
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  out << "<patch version=\"" << SERIALIZER_VERSION << "\" xmlns=\"http://www.novell.com/metadata/zypp/xml-store\">" << endl;

  // reuse Resolvable information serialize function
  out << toXML(static_cast<Resolvable::constPtr>(obj));
  out << toXML(static_cast<ResObject::constPtr>(obj));

  out << "<id>" << xml_escape(obj->id()) << "</id>" << endl;
  out << "<timestamp>" << obj->timestamp().asSeconds() << "</timestamp>" << endl;

  out << "<category>" << obj->category() << "</category>" << endl;
  out << "<affects-package-manager>" << ( obj->affects_pkg_manager() ? "true" : "false" ) << "</affects-package-manager>" << endl;
  out << "<reboot-needed>" << ( obj->reboot_needed() ? "true" : "false" ) << "</reboot-needed>" << endl;

  Patch::AtomList at = obj->atoms();
  out << "  <atoms>" << endl;
  for (Patch::AtomList::iterator it = at.begin(); it != at.end(); it++)
  {
    Resolvable::Ptr one_atom = *it;
    out << castedToXML(one_atom) << endl;
  }
  out << "  </atoms>" << endl;
  out << "</patch>" << endl;
  return out.str();
}

/////////////////////////////////////////////////////////////////
} // namespace storage
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
