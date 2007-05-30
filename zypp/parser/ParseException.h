/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/ParseException.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H
#define ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H

#include <iosfwd>
#include <string>

#include "zypp/base/Exception.h"
#include "zypp/base/UserRequestException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseException
    //
    /** */
    class ParseException : public Exception
    {
    public:
      /** Default ctor */
      ParseException();
      /** Ctor */
      ParseException( const std::string & msg_r );
        /** Dtor */
      virtual ~ParseException() throw();
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H
