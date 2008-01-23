/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/store/serialize.h
*
*/
#ifndef DEVEL_DEVEL_DMACVICAR_SERIALIZE_H
#define DEVEL_DEVEL_DMACVICAR_SERIALIZE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "Backend.h"

#include <zypp/Message.h>
#include <zypp/Resolvable.h>
#include <zypp/Patch.h>
#include <zypp/Package.h>
#include <zypp/Script.h>
#include <zypp/Atom.h>
#include <zypp/Message.h>
#include <zypp/Language.h>
#include <zypp/Pattern.h>
#include <zypp/Selection.h>
#include <zypp/Product.h>
#include <zypp/Edition.h>

#define SERIALIZER_VERSION "2.0"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace storage
{ /////////////////////////////////////////////////////////////////

template<class T>
std::string toXML( const T &obj ); //undefined

template<>
std::string toXML( const Edition &edition );

template<>
std::string toXML( const Arch &arch );

template<>
std::string toXML( const Capability &cap );

template<>
std::string toXML( const Capabilities &caps );

template<>
std::string toXML( const CapabilitySet &caps );    

template<>
std::string toXML( const Dependencies &dep );

/**
 * Serialize Resolvable properties
 * NOTE: This wont serialize child classes properties
 * Use castedToXML for that.
 */
template<>
std::string toXML( const Resolvable::constPtr &obj );

/**
 * Serialize ResObject properties
 */
template<>
std::string toXML( const ResObject::constPtr &obj );

/**
 * Serialize properties based in the specific kind of the Resolvable
 */
std::string castedToXML( const Resolvable::constPtr &ret );

/**
 * lack of instrospection sucks
 */
std::string resolvableTypeToString( const Resolvable::constPtr &resolvable, bool plural = false );

/**
 * lack of instrospection sucks
 */
std::string resolvableKindToString( const Resolvable::Kind &kind, bool plural = false );

template<>
std::string toXML( const Package::constPtr &obj );

template<>
std::string toXML( const Script::constPtr &obj );

template<>
std::string toXML( const Message::constPtr &obj );

template<>
std::string toXML( const Patch::constPtr &obj );

template<>
std::string toXML( const Atom::constPtr &obj );

template<>
std::string toXML( const Pattern::constPtr &obj );

template<>
std::string toXML( const Selection::constPtr &obj );

template<>
std::string toXML( const Product::constPtr &obj );

template<>
std::string toXML( const Language::constPtr &obj );

/////////////////////////////////////////////////////////////////
} // namespace storage
	///////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // DEVEL_DEVEL_DMACVICAR_SQLITEBACKEND_H

