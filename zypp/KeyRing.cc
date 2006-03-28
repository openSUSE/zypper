/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/KeyRing.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp/base/String.h"
#include "zypp/KeyRing.h"
#include "zypp/ExternalProgram.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing::Impl
  //
  /** KeyRing implementation. */
  struct KeyRing::Impl
  {
    Impl()
    {}

    Impl( const Pathname keyring )
    {
      _homedir = keyring;
    }

    void importKey( const Pathname &keyfile );

  private:
    //mutable std::map<Locale, std::string> translations;
    Pathname _homedir;
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  
  void KeyRing::Impl::importKey( const Pathname &keyfile)
  {
    const char* argv[] =
    {
      "gpg",
      "--homedir",
      _homedir.asString().c_str(),
      "--import",
      keyfile.asString().c_str(),
      NULL
    };
    
    ExternalProgram prog(argv,ExternalProgram::Discard_Stderr, false, -1, true);

    //if(!prog)
    //  return 2;
    
    prog.close();
  }
  
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : KeyRing
  //
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::KeyRing
  //	METHOD TYPE : Ctor
  //
  KeyRing::KeyRing()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::KeyRing
  //	METHOD TYPE : Ctor
  //
  KeyRing::KeyRing( const Pathname &keyring )
  : _pimpl( new Impl(keyring) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : KeyRing::~KeyRing
  //	METHOD TYPE : Dtor
  //
  KeyRing::~KeyRing()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////

  void KeyRing::importKey( const Pathname &keyfile)
  {
    _pimpl->importKey(keyfile);
  }
  
  /*
  std::string KeyRing::text( const Locale &lang ) const
  { return _pimpl->text( lang ); }

  void KeyRing::setText( const std::string &text, const Locale &lang )
  { _pimpl->setText( text, lang ); }

  std::set<Locale> KeyRing::locales() const
  {
    return _pimpl->locales();
  }

  void KeyRing::setText( const std::list<std::string> &text, const Locale &lang )
  { _pimpl->setText( text, lang ); }

  Locale KeyRing::detectLanguage() const
  { return _pimpl->detectLanguage(); }
  */
  
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
