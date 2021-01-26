/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Exception.cc
 *
*/
#include <iostream>
#include <sstream>

#include <zypp-core/base/Logger.h>
#include <zypp-core/base/LogTools.h>
#include <zypp-core/base/Gettext.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Exception.h>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace exception_detail
  { /////////////////////////////////////////////////////////////////

    std::string CodeLocation::asString() const
    {
      return str::form( "%s(%s):%u",
                        _file.c_str(),
                        _func.c_str(),
                        _line );
    }

    std::ostream & operator<<( std::ostream & str, const CodeLocation & obj )
    { return str << obj.asString(); }

    /////////////////////////////////////////////////////////////////
  } // namespace exception_detail
  ///////////////////////////////////////////////////////////////////

  Exception::Exception()
  {}

  Exception::Exception( const std::string & msg_r )
  : _msg( msg_r )
  {}

  Exception::Exception( std::string && msg_r )
  : _msg( std::move(msg_r) )
  {}

  Exception::Exception( const std::string & msg_r, const Exception & history_r )
  : _msg( msg_r )
  { remember( history_r ); }

  Exception::Exception( std::string && msg_r, const Exception & history_r )
  : _msg( std::move(msg_r) )
  { remember( history_r ); }

  Exception::Exception( const std::string & msg_r, Exception && history_r )
  : _msg( msg_r )
  { remember( std::move(history_r) ); }

  Exception::Exception( std::string && msg_r, Exception && history_r )
  : _msg( std::move(msg_r) )
  { remember( std::move(history_r) ); }

  Exception::~Exception() throw()
  {}

  std::string Exception::asString() const
  {
    std::ostringstream str;
    dumpOn( str );
    return str.str();
  }

  std::string Exception::asUserString() const
  {
    std::ostringstream str;
    dumpOn( str );
    // call gettext to translate the message. This will
    // not work if dumpOn() uses composed messages.
    return _(str.str().c_str());
  }

  std::string Exception::asUserHistory() const
  {
    if ( historyEmpty() )
      return asUserString();

    std::string ret( asUserString() );
    if ( ret.empty() )
      return historyAsString();

    ret += '\n';
    ret += historyAsString();
    return ret;
  }

  void Exception::remember( const Exception & old_r )
  {
    if ( &old_r != this ) // no self-remember
    {
      History newh( old_r._history.begin(), old_r._history.end() );
      newh.push_front( old_r.asUserString() );
      _history.swap( newh );
    }
  }

  void Exception::remember( Exception && old_r )
  {
    if ( &old_r != this ) // no self-remember
    {
      History & newh( old_r._history );	// stealing it
      newh.push_front( old_r.asUserString() );
      _history.swap( newh );
    }
  }

  void Exception::addHistory( const std::string & msg_r )
  { _history.push_front( msg_r ); }

  void Exception::addHistory( std::string && msg_r )
  { _history.push_front( std::move(msg_r) ); }

  std::string Exception::historyAsString() const
  {
    // TranslatorExplanation followed by the list of error messages that lead to this exception
    std::string history( _("History:") );
    std::ostringstream ret;
    dumpRange( ret, historyBegin(), historyEnd(),
               "", history+"\n - ", "\n - ", "\n", "" );
    return ret.str();
  }

  std::ostream & Exception::dumpOn( std::ostream & str ) const
  { return str << _msg; }

  std::ostream & Exception::dumpError( std::ostream & str ) const
  { return dumpOn( str << _where << ": " ); }

  std::ostream & operator<<( std::ostream & str, const Exception & obj )
  { return obj.dumpError( str ); }


  std::string Exception::strErrno( int errno_r )
  { return str::strerror( errno_r ); }

  std::string Exception::strErrno( int errno_r, const std::string & msg_r )
  { return strErrno( errno_r, std::string(msg_r) );  }

  std::string Exception::strErrno( int errno_r, std::string && msg_r )
  {
    msg_r += ": ";
    return msg_r += strErrno( errno_r );
  }

  void Exception::log( const Exception & excpt_r, const CodeLocation & where_r,
                       const char *const prefix_r )
  {
    INT << where_r << " " << prefix_r << " " << excpt_r.asUserHistory() << endl;
  }

  void Exception::log( const char * typename_r, const CodeLocation & where_r,
                       const char *const prefix_r )
  {
    INT << where_r << " " << prefix_r << " exception of type " << typename_r << endl;
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
