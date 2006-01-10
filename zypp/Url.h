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
   * \class Url
   * \brief Url manipulation class.
   *
   * The generic URL (URI) syntax and its components are defined in the
   * RFC3986 (http://rfc.net/rfc3986.html) Section 3, "Syntax Components".
   * The symantics of a URL and its components is defined in the
   * specification of the scheme used in the URL.
   *
   * This class provides methods to access and manipulate generic and
   * common scheme-specific URL components (or using the more general
   * term, URI components).
   *
   * To consider the scheme-specifics of a URL, the Url class contains
   * a reference object pointing to a UrlBase or derived object, that
   * implements the scheme specifics.
   *
   * Using the Url::registerScheme() method, it is possible to register
   * a preconfigured or derived UrlBase object for a specific scheme
   * name. The registered object will be cloned to handle all URL's
   * containing the specified scheme name.
   *
   * \par RFC3986, Syntax Components:
   *
   * The generic URI syntax consists of a hierarchical sequence of
   * components referred to as the scheme, authority, path, query,
   * and fragment.
   *
   * \code
   *    URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
   *
   *    hier-part   = "//" authority path-abempty
   *                / path-absolute
   *                / path-rootless
   *                / path-empty
   * \endcode
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
   * The following are two example URIs and their component parts:
   * \code
   *      foo://example.com:8042/over/there?name=ferret#nose
   *      \_/   \______________/\_________/ \_________/ \__/
   *       |           |            |            |        |
   *    scheme     authority       path        query   fragment
   *       |   _____________________|__
   *      / \ /                        \
   *      urn:example:animal:ferret:nose
   * \endcode
   *
   */
  class Url
  {
  public:
    typedef zypp::url::EEncoding    EEncoding;
    typedef zypp::url::ViewOptions  ViewOptions;

    ~Url();
    Url();

    /**
     * Create a new Url object as shared copy of the given one.
     *
     * \param url The Url object to make a copy of.
     */
    Url(const Url &url);


    /**
     * \brief Construct a Url object from percent-encoded URL string.
     *
     * Parses the \p encodedUrl string using the parseUrl() method
     * and assings the result to the new created object.
     *
     * \param encodedUrl A percent-encoded URL string.
     * \throws A std::invalid_argument exception if parsing fails.
     */
    Url(const std::string &encodedUrl);


    /**
     * \brief Parse a percent-encoded URL string.
     *
     * Trys to parses the given string into generic URL components
     * and created a clone of a scheme-specialized object or a new
     * UrlBase object.
     *
     * \param encodedUrl A percent-encoded URL string.
     * \return           A reference to a (derived) UrlBase object or
     *                   empty reference if the \p encodedUrl string
     *                   does not match the generic URL syntax.
     * \throws A std::invalid_argument exception in case of invalid
     *         URL component is found.
     */
    static url::UrlRef
    parseUrl(const std::string &encodedUrl);


    /**
     * \brief Assigns parsed percent-encoded URL string.
     *
     * Parses \p encodedUrl string using the parseUrl() method
     * and assigns the result to the current object.
     * 
     * \param encodedUrl A percent-encoded URL string.
     * \return A reference to this Url object.
     * \throws A std::invalid_argument exception if parsing fails.
     */
    Url&
    operator = (const std::string &encodedUrl);


    /**
     * \brief Assigns the \p url to the current object.
     *
     * After this operation the current object will be a shared
     * copy of the object specified in the \p url parameter.
     *
     * \param url The Url object to make a copy of.
     * \return A reference to this Url object.
     */
    Url&
    operator = (const Url &url);


    /**
     * \brief Register a scheme-specific implementation.
     *
     * \param scheme  A name of a scheme.
     * \param urlImpl A UrlBase object specialized for this scheme.
     * \return True, if the object claims to implement the scheme.
     */
    static bool
    registerScheme(const std::string &scheme,
                   url::UrlRef       urlImpl);


    /**
     * \return A vector with registered URL scheme names.
     */
    static url::UrlBase::Schemes
    getAllKnownSchemes();


    /**
     * \return A vector with scheme names known by this object.
     */
    url::UrlBase::Schemes
    getKnownSchemes() const;


    /**
     * \brief Verifies specified scheme name.
     *
     * Verifies if the specified \p scheme name is valid (generic
     * scheme name syntax) and if is contained in the list of
     * current objects known scheme names.
     *
     * \return True, if generic scheme name syntax is valid
     *         and if it is known to the current object.
     */
    bool
    isValidScheme(const std::string &scheme) const;


    /**
     * \brief Verifies Url.
     *
     * Verifies if the current object contains a non-empty scheme
     * name. Additional semantical URL checks may be performed by
     * derived UrlBase objects.
     *
     * \return True, if the Url seems to be valid.
     */
    bool
    isValid() const;


    // -----------------
    /**
     * Returns a default string representation of the Url object.
     *
     * By default, a password in the URL will be hidden.
     *
     * \return A default string representation of the Url object.
     */
    std::string
    toString() const;

    /**
     * Returns a string representation of the Url object.
     *
     * To include a password in the resulting Url string,
     * use:
     * \code
     *    url.toString(url::ViewOptions() +
     *                 url::ViewOptions::WITH_PASSWORD);
     * \endcode
     *
     * or its equivalent:
     *
     * \code
     *    url.toString(url::ViewOptions::DEFAULTS +
     *                 url::ViewOptions::WITH_PASSWORD);
     * \endcode
     *
     * \param opts  A combination of view options.
     * \return A string representation of the Url object. 
     */
    std::string
    toString(const ViewOptions &opts) const;


    // -----------------
    /**
     * \return Scheme name of the current Url object.
     */
    std::string
    getScheme() const;


    /**
     * \return The encoded authority component of the URL
     *         ("user:pass@host:port" without leading "//").
     */
    std::string
    getAuthority() const;


    /**
     * \return The encoded path component of the URL
     *         inclusive path parameters if any.
     */
    std::string
    getPathData() const;


    /**
     * \return The encoded query string component of the URL.
     */
    std::string
    getQueryString() const;


    /**
     * \return The encoded fragment component of the URL.
     */
    std::string
    getFragment(EEncoding eflag = zypp::url::E_DECODED) const;


    // -----------------
    /**
     * \param eflag Flag if the usename should be decoded or not.
     * \return The username sub-component from URL-Authority.
     */
    std::string
    getUsername(EEncoding eflag = zypp::url::E_DECODED) const;


    /**
     * \param eflag Flag if the password should be decoded or not.
     * \return The password sub-component from URL-Authority.
     */
    std::string
    getPassword(EEncoding eflag = zypp::url::E_DECODED) const;


    /**
     * \param eflag Flag if the host should be decoded or not.
     * \return The host (hostname or IP) sub-component from URL-Authority.
     */
    std::string
    getHost(EEncoding eflag = zypp::url::E_DECODED) const;

    /**
     * \param eflag Flag if the port should be decoded or not.
     * \return The port sub-component from URL-Authority.
     */
    std::string
    getPort() const;


    /**
     * \param eflag Flag if the path should be decoded or not.
     * \return The path name sub-component without path parameters
     *  from Path-Data of the URL.
     */
    std::string
    getPathName(EEncoding eflag = zypp::url::E_DECODED) const;


    /**
     * \return The encoded path parameters from Path-Data of the URL.
     */
    std::string
    getPathParams() const;


    // -----------------
    /**
     * Returns a vector of path parameter substrings.
     *
     * The default path parameter separator is the \c ',' character.
     * A schema specific object may overide the default separators.
     *
     * For example, the path parameters string "foo=1,bar=2" is splited
     * by default into a vector containing the substrings "foo=1" and
     * "bar=2".
     *
     * \return The path parameters splited into a vector of substrings.
     */
    zypp::url::ParamVec
    getPathParamsVec() const;


    /**
     * Returns a string map with path parameter keys and values.
     *
     * The default path parameter separator is the \c ',' character,
     * the default key/value separator for the path parameters is
     * the \c '=' character.
     * A schema specific object may overide the default separators.
     *
     * For example, the path parameters string "foo=1,bar=2" is splited
     * into a map containing "foo" = "1" and "bar" = "2" by default.
     *
     * \param eflag Flag if the path parameter keys and values should
     *               be decoded or not.
     * \return The path parameters key and values as a string map.
     */
    zypp::url::ParamMap
    getPathParamsMap(EEncoding eflag = zypp::url::E_DECODED) const;


    /**
     * Return the value for the specified path parameter key.
     *
     * For example, if the path parameters string is "foo=1,bar=2"
     * the method will return the substring "1" for the param key
     * "foo" and "2" for the param key "bar".
     *
     * \param param The path parameter key.
     * \param eflag Flag if the path parameter keys and values should
     *              be decoded or not.
     * \return The value for the path parameter key or empty string.
     */
    std::string
    getPathParam(const std::string &param,
                 EEncoding eflag = zypp::url::E_DECODED) const;


    /**
     * Returns a vector of query string parameter substrings.
     *
     * The default query string parameter separator is the \c '&'
     * character.
     * A schema specific object may overide the default separators.
     *
     * For example, the query string "foo=1&bar=2" is splited by
     * default into a vector containing the substrings "foo=1" and
     * "bar=2".
     *
     * \return The query string splited into a vector of substrings.
     */
    zypp::url::ParamVec
    getQueryStringVec() const;

    /**
     * Returns a string map with query string keys and values.
     *
     * The default query string parameter separator is the \c ','
     * character, the default key/value separator the \c '=' character.
     * A schema specific object may overide the default separators.
     *
     * For example, the query string "foo=1&bar=2" is splited by
     * default into a map containing "foo" = "1" and "bar" = "2".
     *
     * \param eflag Flag if the query string keys and values should
     *               be decoded or not.
     * \return The query string as a key/value string map.
     */
    zypp::url::ParamMap
    getQueryStringMap(EEncoding eflag = zypp::url::E_DECODED) const;

    /**
     * Return the value for the specified query parameter key.
     *
     * For example, if the query string is "foo=1,bar=2" the method
     * will return the substring "1" for the param key "foo" and
     * "2" for the param key "bar".
     *
     * \param param The query parameter key.
     * \param eflag Flag if the query parameter keys and values should
     *              be decoded or not.
     * \return The value for the query parameter key or empty string.
     */
    std::string
    getQueryParam(const std::string &param,
                  EEncoding eflag = zypp::url::E_DECODED) const;


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
    setPathParam(const std::string &param, const std::string &value);

    void
    setPathParamsVec(const zypp::url::ParamVec &pvec);

    void
    setPathParamsMap(const zypp::url::ParamMap &pmap);

    void
    setQueryParam(const std::string &param, const std::string &value);

    void
    setQueryStringVec(const zypp::url::ParamVec &pvec);

    void
    setQueryStringMap(const zypp::url::ParamMap &pmap);

  private:
    url::UrlRef m_impl;
  };

  std::ostream & operator<<( std::ostream & str, const Url & url );

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_URL_H */
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
