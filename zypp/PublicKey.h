/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PublicKey.h
 *
*/
#ifndef ZYPP_PUBLICKEY_H
#define ZYPP_PUBLICKEY_H

#include <iosfwd>
#include <map>
#include <list>
#include <set>
#include <string>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace devel
{
  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PublicKey
  //
  /** Class that represent a GPG Public Key
  */
  class PublicKey
  {
    friend std::ostream & operator<<( std::ostream & str, const PublicKey & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    PublicKey();
   /** Ctor 
    * \throws when data does not make a key
    */
    PublicKey(const Pathname &file);
    ~PublicKey();
    
    std::string asString() const;
    std::string armoredData() const;
    std::string id() const;
    std::string name() const;
    std::string fingerprint() const;
    
  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PublicKey Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PublicKey & obj )
  { return str << obj.asString(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
}
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PUBLICKEY_H
