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
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * Exception thrown when the supplied key is
   * not a valid gpg key
   */
  class BadKeyException : public Exception
  {
    public:
      /** Ctor taking message.
     * Use \ref ZYPP_THROW to throw exceptions.
       */
      BadKeyException()
      : Exception( "Bad Key Exception" )
      {}

      Pathname keyFile() const
      { return _keyfile; }

      /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
       */
      BadKeyException( const std::string & msg_r, const Pathname &keyfile = Pathname() )
      : Exception( msg_r ), _keyfile(keyfile)
      {}
      /** Dtor. */
      virtual ~BadKeyException() throw() {};
    private:
      Pathname _keyfile;
  };


  // forward declaration of class Date
  class Date;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PublicKey
  //
  /**
   * Class that represent a GPG Public Key
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

    bool isValid() const
    { return ( ! id().empty() && ! fingerprint().empty() && !path().empty() ); }

    std::string asString() const;
    std::string armoredData() const;
    std::string id() const;
    std::string name() const;
    std::string fingerprint() const;

    /**
     * Date when the key was created (time is 00:00:00)
     */
    Date created() const;

    /**
     * Date when the key expires (time is 00:00:00)
     * If the key never expires the date is Date() (i.e. 0 seconds since the epoch (1.1.1970))
     */
    Date expires() const;

    Pathname path() const;

    bool operator==( PublicKey b ) const;
    bool operator==( std::string sid ) const;

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PublicKey Stream output */
  inline std::ostream & operator<<( std::ostream & str, const PublicKey & obj )
  { return str << obj.asString(); }

  /** \relates PublicKey Detailed stream output */
  std::ostream & dumpOn( std::ostream & str, const PublicKey & obj );

 /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PUBLICKEY_H
