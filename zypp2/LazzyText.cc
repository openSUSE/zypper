/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/LazzyText.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"
#include <fstream>
#include "zypp/PathInfo.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"
#include "zypp2/LazzyText.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LazzyText::Impl
  //
  /** LazzyText implementation. */
  struct LazzyText::Impl
  {
    enum Location
    {
      InMemory,
      OnDisk
    };
    
    Impl()
      : _location(InMemory)
    {}

    Impl( const Pathname &path, long int start, long int offset )
          : _location(OnDisk), _path(path), _start(start), _offset(offset)
    {
    
    }
        
    Impl( const std::string &value )
       : _location(InMemory), _value(value)
    {
        
    }

    operator std::string() const
    { return _value; }
        
//     TagData & operator=( const std::string &s )
//     {
//       _location = InMemory;
//       _value = s;
//       return *this;
//     }
        
    std::string text() const
    {
      if ( _location == OnDisk )
      {
        char buffer[_offset + 1];
        std::ifstream is( _path.c_str() );
        
        if ( ! is )
        {
          ZYPP_THROW( Exception("Can't open " + _path.asString()) );
        }
        
        is.seekg(_start);
        is.read( buffer, _offset );
        buffer[_offset] = '\0';
        return std::string(buffer);
       }
       else
       {
          return _value;
       }
     }
  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

  protected:
    Location _location;
    Pathname _path;
    long int _start;
    long int _offset;
    std::string _value;
  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LazzyText::LazzyText
  //	METHOD TYPE : Ctor
  //
  LazzyText::LazzyText()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LazzyText::LazzyText
  //	METHOD TYPE : Ctor
  //
  LazzyText::LazzyText( const Pathname &path, long int start, long int offset )
  : _pimpl( new Impl(path, start, offset) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LazzyText::LazzyText
  //	METHOD TYPE : Ctor
  //
  LazzyText::LazzyText( const std::string &s )
  : _pimpl( new Impl(s) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LazzyText::~LazzyText
  //	METHOD TYPE : Dtor
  //
  LazzyText::~LazzyText()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to implementation:
  //
  ///////////////////////////////////////////////////////////////////

  std::string LazzyText::text() const
  { return _pimpl->text(); }
  
  std::string LazzyText::asString() const
  { return _pimpl->text(); }

  bool LazzyText::empty() const
  { return _pimpl->text().empty(); }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
