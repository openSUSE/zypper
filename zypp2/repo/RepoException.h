/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/RepoException.h
 *
*/
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
  namespace repo
  { /////////////////////////////////////////////////////////////////

    /**
     * \short Exception for repository handling.
     */
    class RepoException : public Exception
    {
    public:
      /** Default ctor */
      RepoException();
      /** Ctor */
      RepoException( const std::string & msg_r );
        /** Dtor */
      virtual ~RepoException() throw();
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    };
    ///////////////////////////////////////////////////////////////////

    /**
     * The repository cache is not built yet
     * so you can't create the repostories from
     * the cache.
     */
    class RepoNotCachedException : public RepoException
    {
    public:
      RepoNotCachedException();
      RepoNotCachedException( const std::string & msg_r );
      virtual ~RepoNotCachedException() throw();
    };
    
    /**
     * thrown when it was impossible to
     * determine one url for this repo.
     */
    class RepoNoUrlException : public RepoException
    {
    
    };
    
    /**
     * thrown when it was impossible to
     * determine an alias for this repo.
     */
    class RepoNoAliasException : public RepoException
    {
    
    };
    
    /**
     * thrown when it was impossible to
     * match a repository
     */
    class RepoNotFoundException : public RepoException
    {
    
    };
    
    /**
     * Repository already exists and some unique
     * attribute can't be duplicated.
     */
    class RepoAlreadyExistsException : public RepoException
    {
    
    };
    
    /**
     * thrown when it was impossible to
     * determine an alias for this repo.
     */
    class RepoUnknownTypeException : public RepoException
    {
    
    };
    
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_PARSEEXCEPTION_H
