/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KeyRing.h
 *
*/
#ifndef ZYPP_KEYRING_H
#define ZYPP_KEYRING_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Locale.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct PublicKey
  {
    std::string id;
    std::string name;
    std::string fingerprint;
  };
  
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing
  //
  /** Class that represent a text and multiple translations.
  */
  class KeyRing
  {
    friend std::ostream & operator<<( std::ostream & str, const KeyRing & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    KeyRing();
    /** Ctor \todo Make ctor it explicit */
    explicit
    KeyRing(const Pathname &general_kr, const Pathname &trusted_kr);
    /**
     * imports a key from a file and returns the imported key.
     * throw if key was not imported
     */
    PublicKey importKey( const Pathname &keyfile, bool trusted = false);
    /**
     * removes a key from the keyring. 
     * If trusted is true, Remove it from trusted keyring too.
     */
    void deleteKey( const std::string &id, bool trusted =  false);
    
    std::list<PublicKey> publicKeys();
    std::list<PublicKey> trustedPublicKeys();
    
/** Dtor */
    ~KeyRing();

  public:

    /** Synonym for \ref text */
    //std::string asString() const
    //{}

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates KeyRing Stream output */
  inline std::ostream & operator<<( std::ostream & str, const KeyRing & obj )
  {
    //return str << obj.asString();
    return str;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_KEYRING_H
