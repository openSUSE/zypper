/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/WebpinResult.h
 *
*/
#ifndef ZYPP_WEBPINRESULT_H
#define ZYPP_WEBPINRESULT_H

#include <iosfwd>
#include <list>
#include <set>
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"
#include "zypp/APIConfig.h"

#include "zypp/CheckSum.h"
#include "zypp/Edition.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/repo/RepoType.h"
#include "zypp/repo/RepoVariables.h"

namespace zypp
{ 
namespace ws
{
  /**
   * \short Represents a result from 
   * http://api.opensuse-community.org/searchservice/Search
   * web service
   *
   */
  class WebpinResult
  {
    friend std::ostream & operator<<( std::ostream & str, const WebpinResult & obj );

    public:
    WebpinResult();
    ~WebpinResult();

    /**
     * package name
     */
    std::string name() const;

    /**
     * set the package name \see name
     * \param name
     */
    WebpinResult & setName( const std::string &name );

    /**
     * package edition
     */
    zypp::Edition edition() const;

    /**
     * set the package edition \see edition
     * \param edition
     */
    WebpinResult & setEdition( const zypp::Edition &name );

    /**
     * repository's url
     * The url of the repository where this package
     * is located
     */
    zypp::Url repositoryUrl() const;

    /**
     * set the repository url where this package comes from
     * \see repositoryUrl
     * \param url
     */
    WebpinResult & setRepositoryUrl( const zypp::Url &url );
      
   /**
     * package priority
     */
    int priority() const;

    /**
     * set the package priority \see priority
     * \param priority
     */
    WebpinResult & setPriority( int priority );


    /**
     * package summary
     */
    std::string summary() const;

    /**
     * set the package summary \see summary
     * \param summary
     */
    WebpinResult & setSummary( const std::string &summary );

    /**
     * package distribution
     * Example: openSUSE 10.3
     */
    std::string distribution() const;

    /**
     * set the package distribution \see distribution
     * \param distribution
     */
    WebpinResult & setDistribution( const std::string &distribution );

   /**
     * package checksum
     * Example: a md5sum or sha1sum
     */
    zypp::CheckSum checksum() const;

    /**
     * set the package checksum \see checksum
     * \param checksum
     */
    WebpinResult & setChecksum( const zypp::CheckSum &checksum );


    std::ostream & dumpOn( std::ostream & str ) const;

    class Impl;
  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates RepoInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const WebpinResult & obj );

} // namespace ws
} // namespace zypp


#endif // ZYPP_WEBPINRESULT_H
