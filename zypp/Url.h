/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/Url.h
 */
#ifndef   ZYPP_URL_H
#define   ZYPP_URL_H

#include <zypp/url/UrlBase.h>
#include <zypp/url/UrlUtils.h>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////


  /**
   * @class Url
   * @brief Url handling class implementation.
   *
   *
   * @par From RFC3986, 3. Syntax Components:
   *
   * The generic URI syntax consists of a hierarchical sequence of
   * components referred to as the scheme, authority, path, query,
   * and fragment.
   *
   * @code
   *    URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
   *
   *    hier-part   = "//" authority path-abempty
   *                / path-absolute
   *                / path-rootless
   *                / path-empty
   * @endcode
   *
   * The scheme and path components are required, though the path may be
   * empty (no characters).
   * When authority is present, the path must either be empty or begin
   * with a slash ("/") character.
   * When authority is not present, the path cannot begin with two slash
   * characters ("//").
   * These restrictions result in five different ABNF rules for a path
   * (Section 3.3), only one of which will match any given URI reference.
   *
   * @code
   * The following are two example URIs and their component parts:
   *
   *      foo://example.com:8042/over/there?name=ferret#nose
   *      \_/   \______________/\_________/ \_________/ \__/
   *       |           |            |            |        |
   *    scheme     authority       path        query   fragment
   *       |   _____________________|__
   *      / \ /                        \
   *      urn:example:animal:ferret:nose
   * @endcode
   */
  class Url
  {
  public:
    typedef zypp::url::EEncoding  EEncoding;

    Url();
    Url(const Url &url);
    Url(const std::string &encodedUrl);
    ~Url();

    static bool
    registerScheme(const std::string &scheme,
                   url::UrlRef       urlImpl);

    static url::UrlRef
    parseUrl(const std::string &encodedUrl);


    static url::UrlBase::Schemes
    getAllKnownSchemes();

    url::UrlBase::Schemes
    getKnownSchemes() const;

    bool
    isValidScheme(const std::string &scheme) const;

    // -----------------
    // TODO: hide pass, ...
    std::string
    toString() const;


    // -----------------
    std::string
    getScheme() const;

    std::string
    getAuthority() const;

    std::string
    getPathData() const;

    std::string
    getQueryString() const;

    std::string
    getFragment(EEncoding eflag = zypp::url::E_DECODED) const;


    // -----------------
    std::string
    getUsername(EEncoding eflag = zypp::url::E_DECODED) const;

    std::string
    getPassword(EEncoding eflag = zypp::url::E_DECODED) const;

    std::string
    getHost(EEncoding eflag = zypp::url::E_DECODED) const;

    std::string
    getPort() const;

    std::string
    getPathName(EEncoding eflag = zypp::url::E_DECODED) const;

    std::string
    getPathParams() const;


    // -----------------
    zypp::url::ParamVec
    getPathParamsVec() const;

    zypp::url::ParamMap
    getPathParamsMap(EEncoding eflag = zypp::url::E_DECODED) const;

    zypp::url::ParamVec
    getQueryStringVec() const;

    zypp::url::ParamMap
    getQueryStringMap(EEncoding eflag = zypp::url::E_DECODED) const;


    // -----------------
    void
    setScheme(const std::string &scheme);

    void
    setAuthority(const std::string &authority);

    void
    setPathData(const std::string &pathdata);

    void
    setQueryString(const std::string &querystr);

    void
    setFragment(const std::string &fragment);


    // -----------------
    void
    setUsername(const std::string &user);

    void
    setPassword(const std::string &pass);

    void
    setHost(const std::string &host);

    void
    setPort(const std::string &port);

    void
    setPathName(const std::string &path);

    void
    setPathParams(const std::string &params);


    // -----------------
    void
    setPathParamsVec(const zypp::url::ParamVec &pvec);

    void
    setPathParamsMap(const zypp::url::ParamMap &pmap);

    void
    setQueryStringVec(const zypp::url::ParamVec &pvec);

    void
    setQueryStringMap(const zypp::url::ParamMap &pmap);

  private:
    url::UrlRef m_impl;
  };


  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_URL_H */
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
