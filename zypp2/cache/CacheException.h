/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_REPOEXCEPTION_H
#define ZYPP_REPO_REPOEXCEPTION_H

#include <iosfwd>
#include <string>

#include "zypp/base/Exception.h"
#include "zypp/base/UserRequestException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    /**
     * \short Exception for cache errors
     */
    class CacheException : public Exception
    {
    public:
      /** Default ctor */
      CacheException();
      /** Ctor */
      CacheException( const std::string & msg_r );
        /** Dtor */
      virtual ~CacheException() throw();
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    };
    ///////////////////////////////////////////////////////////////////

    /**
     * The record you supplied can't be found
     */
    class CacheRecordNotFoundException : public CacheException
    {
    
    };
    
    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H
