/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/url/UrlBase.h
 */
#ifndef   ZYPP_URL_URLBASE_H
#define   ZYPP_URL_URLBASE_H

#include <zypp/url/UrlUtils.h>
#include <zypp/base/PtrTypes.h>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    /**
     * UrlBase behaviour configuration variables container.
     */
    typedef std::map< std::string, std::string > UrlConfig;


    // ---------------------------------------------------------------
    /**
     * Vector of URL scheme names.
     */
    typedef std::vector<std::string>             UrlSchemes;


    // ---------------------------------------------------------------
    /**
     * Url::toString() view options.
     *
     * A instance of this class represents a bit-wise combination
     * of view option constants.
     *
     * It provides ViewOption::operator+() and ViewOption::operator-()
     * to modify a view option combination and a ViewOption::has()
     * method, to check if a specified option is enabled or not.
     */
    struct ViewOption
    {
      /** @{ */
      /**
       * Option to include scheme name in the URL string.
       *
       * Disabling this option causes, that the URL string
       * contains the path, query and fragment components
       * only, for example just "/foo/bar.txt".
       *
       * This option is \b enabled by default.
       */
      static const ViewOption WITH_SCHEME;
      /**
       * Option to include username in the URL string.
       *
       * This option depends on a enabled WITH_SCHEME and
       * WITH_HOST options and is \b enabled by default.
       */
      static const ViewOption WITH_USERNAME;
      /**
       * Option to include password in the URL string.
       *
       * This option depends on a enabled WITH_SCHEME,
       * WITH_HOST and WITH_USERNAME options and is
       * \b disabled by default, causing to hide the
       * password in the URL authority.
       */
      static const ViewOption WITH_PASSWORD;
      /**
       * Option to include hostname in the URL string.
       *
       * This option depends on a enabled WITH_SCHEME
       * option and is \b enabled by default.
       */
      static const ViewOption WITH_HOST;
      /**
       * Option to include port number in the URL string.
       *
       * This option depends on a enabled WITH_SCHEME and
       * WITH_HOST options and is \b enabled by default.
       */
      static const ViewOption WITH_PORT;
      /**
       * Option to include path name in the URL string.
       *
       * This option is \b enabled by default.
       */
      static const ViewOption WITH_PATH_NAME;
      /**
       * Option to include path parameters in the URL string.
       *
       * This option depends on a enabled WITH_PATH_NAME
       * option and is \b disabled by default, causing to
       * hide the path parameters.
       */
      static const ViewOption WITH_PATH_PARAMS;
      /**
       * Option to include query string in the URL string.
       *
       * This option is \b enabled by default.
       */
      static const ViewOption WITH_QUERY_STR;
      /**
       * Option to include fragment string in the URL string.
       *
       * This option is \b enabled by default.
       */
      static const ViewOption WITH_FRAGMENT;
      /** @} */

      /** @{ */
      /**
       * Explicitely include the URL authority separator "//".
       *
       * It causes, that the URL string includes an empty URL
       * authority, for example:
       * "file:///foo.txt" instead of just "file:/foo.txt".
       *
       * This option depends on a enabled WITH_SCHEME view
       * option and is enabled by default.
       */
      static const ViewOption EMPTY_AUTHORITY;
      /**
       * Explicitely include the "/" path character.
       *
       * It causes, that a "/" is added to the Url if the path
       * name is empty, for example:
       *
       * "http://localhost/" instead of just "http://localhost".
       *
       * This option depends on a enabled WITH_PATH_NAME view
       * option and is enabled by default.
       */
      static const ViewOption EMPTY_PATH_NAME;
      /**
       * Explicitely include the path parameters separator ";".
       *
       * It causes, that the URL allways contains the ";" path
       * parameters separator.
       *
       * This option depends on a enabled EMPTY_PATH_NAME view
       * option and is disabled by default.
       */
      static const ViewOption EMPTY_PATH_PARAMS;
      /**
       * Explicitely include the query string separator "?".
       *
       * It causes, that if the query string is requested using
       * the WITH_QUERY_STR option, the URL allways contains the
       * "?" query string separator, even if the query string is
       * empty.
       * This option depends on a enabled WITH_QUERY_STR view
       * option and is disabled by default.
       */
      static const ViewOption EMPTY_QUERY_STR;
      /**
       * Explicitely include the fragment string separator "#".
       *
       * It causes, that if the fragment string is requested using
       * the WITH_FRAGMENT option, the URL allways contains the "#"
       * fragment string separator, even if the fragment string is
       * empty. 
       * This option depends on a enabled WITH_FRAGMENT view
       * option and is disabled by default.
       */
      static const ViewOption EMPTY_FRAGMENT;
      /** @} */

      /** @{ */
      /**
       * Default combination of view options.
       *
       * By default, following view options are enabled:
       *   WITH_SCHEME,    WITH_USERNAME,    WITH_HOST,
       *   WITH_PORT,      WITH_PATH_NAME,   WITH_QUERY_STR,
       *   WITH_FRAGMENT,  EMPTY_AUTHORITY,  EMPTY_PATH_NAME.
       */
      static const ViewOption DEFAULTS;
      /** @} */

      /**
       * Create instance with default combination of view options.
       */
      ViewOption(): opt(DEFAULTS.opt)
      {}

      /**
       * Adds \p l and \p r to a new option combination.
       *
       * @return The new option combination.
       */
      friend inline ViewOption
      operator + (const ViewOption &l, const ViewOption &r)
      { return ViewOption(l.opt |  r.opt); }

      /**
       * Substract \p r from \p l to a new option combination.
       *
       * @return The new option combination.
       */
      friend inline ViewOption
      operator - (const ViewOption &l, const ViewOption &r)
      { return ViewOption(l.opt & ~r.opt); }

      /**
       * Assign specified option combination \p o to the current object.
       *
       * \param o   The option or option combination to make a copy of.
       * \return A reference to this option combination.
       */
      inline ViewOption &
      operator = (const ViewOption &o)
      { opt = o.opt; return *this; }

      /**
       * Check if specified option \p o is set in the current object.
       * \param o    A view option constant.
       * \return True, if specified option \p o is
       *               set/enabled in the instance.
       */
      inline bool
      has(const ViewOption &o) const
      { return o.opt & opt; }

    private:
      ViewOption(int o): opt(o) {}
      int opt;
    };


    // ---------------------------------------------------------------
    /**
     * ViewOptions is just an alias for ViewOption.
     */
    typedef ViewOption ViewOptions;


    // ---------------------------------------------------------------
    /**
     * Internal data as used by UrlBase.
     */
    class UrlData
    {
    public:
      UrlData()
      {}

      UrlData(const UrlConfig &conf)
        : config(conf)
      {}

      UrlConfig       config;
      ViewOptions     vopts;

      std::string     scheme;
      std::string     user;
      std::string     pass;
      std::string     host;
      std::string     port;
      std::string     pathname;
      std::string     pathparams;
      std::string     querystr;
      std::string     fragment;
    };


    // ---------------------------------------------------------------
    /**
     * \brief Generic Url base class.
     *
     * The UrlBase class implements default behaviour for URL
     * manipulations and a base for implementation of scheme-
     * specialized URL's for the Url class.
     *
     */
    class UrlBase
    {
    public:

      virtual ~UrlBase();
      UrlBase();

      /**
       * Create a new Url object as copy of the given one.
       * \param url The Url object to make a copy of.
       */
      UrlBase(const UrlBase &url);

      /**
       * \brief Construct new object and initializes it with
       *        specified URL components.
       *
       * \param scheme    A scheme name.
       * \param authority A authority component data (encoded).
       * \param pathdata  A path component data (encoded).
       * \param querystr  A query string (encoded).
       * \param fragment  A fragment component (encoded),
       */
      UrlBase(const std::string &scheme,
              const std::string &authority,
              const std::string &pathdata,
              const std::string &querystr,
              const std::string &fragment);


      // -----------------
      /**
       * \brief Clears all data in the object.
       */
      virtual void
      clear();

      /**
       * Returns pointer to a copy of the current object.
       *
       * Should be reimplemented by all derived object using
       * the copy constructor of the derived class, e.g.:
       * \code
       *   return new MyUrlDerivedFromUrlBase(*this);
       * \endcode
       *
       * \return A pointer to a copy of the current object.
       */
      virtual UrlBase *
      clone() const;

      /**
       * \brief Initializes current object with new URL components.
       *
       * \param scheme    A scheme name.
       * \param authority A authority component data (encoded).
       * \param pathdata  A path component data (encoded).
       * \param querystr  A query string (encoded).
       * \param fragment  A fragment component (encoded),
       */
      virtual void
      init(const std::string &scheme,
           const std::string &authority,
           const std::string &pathdata,
           const std::string &querystr,
           const std::string &fragment);


      // -----------------
      /**
       * \brief Returns scheme names known by this object.
       *
       * This method is used in the isValidScheme() method and
       * is intended to be reimplemented by derived classes to
       * return the scheme names it implements (is restricted
       * or compatible to).
       *
       * For example, if your derived class implements special
       * features of LDAP URL's, this method may return "ldap"
       * and "ldaps" scheme names.
       *
       * The UrlBase class returns an empty vector, that signals
       * that it is useable with all URL's.
       *
       * \return A vector with scheme names known by this object.
       */
      virtual UrlSchemes
      getKnownSchemes() const;

      /**
       * \brief Returns if scheme name is known to this object.
       * \return True, if scheme name is known to this object.
       */
      virtual bool
      isKnownScheme(const std::string &scheme) const;


      /**
       * \brief Verifies specified scheme name.
       *
       * Verifies the generic syntax of the specified \p scheme name
       * and if it is contained in the current object's list of known
       * schemes (see getKnownSchemes()) if the list is not empty (as
       * in the UrlBase class).
       *
       * \return True, if generic scheme name syntax is valid and
       *         the scheme name is known to the current object.
       */
      virtual bool
      isValidScheme(const std::string &scheme) const;


      /**
       * \brief Verifies the Url.
       *
       * Verifies if the current object contains a non-empty scheme
       * name. Additional semantical URL checks may be performed by
       * derived UrlBase-objects.
       *
       * \return True, if the Url seems to be valid.
       */
      virtual bool
      isValid() const;


      // -----------------
      /**
       * Returns a default string representation of the Url object.
       *
       * By default, a password in the URL will be hidden.
       *
       * \return A default string representation of the Url object.
       */
      virtual std::string
      toString() const;

      /**
       * Returns a string representation of the Url object.
       *
       * To include a password in the resulting Url string, use:
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
      virtual std::string
      toString(const zypp::url::ViewOptions &opts) const;


      // -----------------
      /**
       * Returns the scheme name of the URL.
       * \return Scheme name of the current Url object.
       */
      virtual std::string
      getScheme() const;


      // -----------------
      /**
       * Returns the encoded authority component of the URL.
       *
       * The returned authority string does not contain the leading
       * "//" separator characters, but just its "user:pass@host:port"
       * content only.
       *
       * \return The encoded authority component string.
       */
      virtual std::string
      getAuthority() const;

      /**
       * Returns the username from the URL authority.
       * \param eflag Flag if the usename should be percent-decoded or not.
       * \return The username sub-component from the URL authority.
       */
      virtual std::string
      getUsername(EEncoding eflag) const;

      /**
       * Returns the password from the URL authority.
       * \param eflag Flag if the password should be percent-decoded or not.
       * \return The password sub-component from the URL authority.
       */
      virtual std::string
      getPassword(EEncoding eflag) const;

      /**
       * Returns the hostname or IP from the URL authority.
       *
       * In case the Url contains an IP number, it may be surrounded
       * by "[" and "]" characters, for example "[::1]" for an IPv6
       * localhost address.
       *
       * \param eflag Flag if the host should be percent-decoded or not.
       * \return The host sub-component from the URL authority.
       */
      virtual std::string
      getHost(EEncoding eflag) const;

      /**
       * Returns the port from the URL authority.
       * \return The port sub-component from the URL authority.
       */
      virtual std::string
      getPort() const;


      // -----------------
      /**
       * Returns the encoded path component of the URL.
       *
       * The path data contains the path name, optionally
       * followed by path parameters separated with a ";"
       * character, for example "/foo/bar;version=1.1".
       *
       * \return The encoded path component of the URL.
       */
      virtual std::string
      getPathData() const;

      /**
       * Returns the path name from the URL.
       * \param eflag Flag if the path should be decoded or not.
       * \return The path name sub-component without path parameters
       *         from Path-Data component of the URL.
       */
      virtual std::string
      getPathName(EEncoding eflag) const;

      /**
       * Returns the path parameters from the URL.
       * \return The encoded path parameters from the URL.
       */
      virtual std::string
      getPathParams() const;

      /**
       * Returns a vector with path parameter substrings.
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
      virtual zypp::url::ParamVec
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
      virtual zypp::url::ParamMap
      getPathParamsMap(EEncoding eflag) const;

      /**
       * Return the value for the specified path parameter.
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
      virtual std::string
      getPathParam(const std::string &param, EEncoding eflag) const;


      // -----------------
      /**
       * Returns the encoded query string component of the URL.
       *
       * The query string is returned without first "?" (separator)
       * character. Further "?" characters as in e.g. LDAP URL's
       * remains in the returned string.
       *
       * \return The encoded query string component of the URL.
       */
      virtual std::string
      getQueryString() const;

      /**
       * Returns a vector with query string parameter substrings.
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
      virtual zypp::url::ParamVec
      getQueryStringVec() const;

      /**
       * Returns a string map with query parameter and their values.
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
      virtual zypp::url::ParamMap
      getQueryStringMap(EEncoding eflag) const;

      /**
       * Return the value for the specified query parameter.
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
      virtual std::string
      getQueryParam(const std::string &param, EEncoding eflag) const;


      // -----------------
      /**
       * Returns the encoded fragment component of the URL.
       * \param eflag Flag if the fragment should be percent-decoded or not.
       * \return The encoded fragment component of the URL.
       */
      virtual std::string
      getFragment(EEncoding eflag) const;


      // -----------------
      /**
       * \brief Set the scheme name in the URL.
       * \param scheme The new scheme name.
       */
      virtual void
      setScheme(const std::string &scheme);


      // -----------------
      /**
       * \brief Set the authority component in the URL.
       *
       * The \p authority string shoud not contain any leading
       * "//" separator characters (just "user:pass@host:port").
       *
       * \param authority The authority component string.
       */
      virtual void
      setAuthority(const std::string &authority);

      /**
       * \brief Set the username in the URL authority.
       * \param user The new username.
       */
      virtual void
      setUsername(const std::string &user);

      /**
       * \brief Set the password in the URL authority.
       * \param pass The new password.
       */
      virtual void
      setPassword(const std::string &pass);

      /**
       * \brief Set the hostname or IP in the URL authority.
       * \param host The new hostname or IP.
       */
      virtual void
      setHost(const std::string &host);

      /**
       * \brief Set the port number in the URL authority.
       * \param port The new port number.
       */
      virtual void
      setPort(const std::string &port);


      // -----------------
      /**
       * \brief Set the path data component in the URL.
       *
       * The \p pathdata string may include path parameters
       * separated using the ";" separator character.
       *
       * \param pathdata The encoded path data component string.
       */
      virtual void
      setPathData(const std::string &pathdata);

      /**
       * \brief Set the path name.
       * \param path The new path name.
       */
      virtual void
      setPathName(const std::string &path);

      /**
       * \brief Set the path parameters.
       * \param params The new path parameter string.
       */
      virtual void
      setPathParams(const std::string &params);

      /**
       * \brief Set the path parameters.
       * \param pvec The vector with path parameters.
       */
      virtual void
      setPathParamsVec(const zypp::url::ParamVec &pvec);

      /**
       * \brief Set the path parameters.
       * \param pmap The map with path parameters.
       */
      virtual void
      setPathParamsMap(const zypp::url::ParamMap &pmap);

      /**
       * \brief Set or add value for the specified path parameter.
       * \param param The path parameter name.
       * \param value The path parameter value.
       */
      virtual void
      setPathParam(const std::string &param, const std::string &value);


      // -----------------
      /**
       * \brief Set the query string in the URL.
       *
       * The \p querystr string will be supposed to not to
       * contain the "?" separator character.
       *
       * \param querystr The new encoded query string.
       */
      virtual void
      setQueryString(const std::string &querystr);

      /**
       * \brief Set the query parameters.
       * \param qvec The vector with query parameters.
       */
      virtual void
      setQueryStringVec(const zypp::url::ParamVec &qvec);

      /**
       * \brief Set the query parameters.
       * \param qmap The map with query parameters.
       */
      virtual void
      setQueryStringMap(const zypp::url::ParamMap &qmap);

      /**
       * \brief Set or add value for the specified query parameter.
       * \param param The query parameter name.
       * \param value The query parameter value.
       */
      virtual void
      setQueryParam(const std::string &param, const std::string &value);


      // -----------------
      /**
       * \brief Set the fragment string in the URL.
       * \param fragment The new encoded fragment string.
       */
      virtual void
      setFragment(const std::string &fragment);


      // -----------------
      /**
       * Configures behaviour of the instance.
       *
       * This method is called in UrlBase constructors before
       * any URL components are applied.
       * Derived classes may reimplement this method to change
       * the behaviour of the object.
       * Use the config() methods to query and change them.
       *
       * The UrlBase class uses following config variables:
       *
       * - Path parameter separators:
       *   - \a \c sep_pathparams   \c ";"
       *     Separator used to split path parameters from path name.
       *   - \a \c psep_pathparam   \c ","
       *     Separator between path parameters.
       *   - \a \c vsep_pathparam   \c "="
       *     Separator between key and value of a path parameter.
       *   .
       * .
       *
       * - Query string separators:
       *   - \a \c psep_querystr    \c "&"
       *     Separator between query string parameters.
       *   - \a \c vsep_querystr    \c "="
       *     Separator between key and value of a query parameter.
       *   .
       * .
       *
       * - Characters in URL components, that are safe without
       *   URL percent-encoding (see zypp::url::encode()).
       *   - \a safe_username    ""
       *   - \a safe_password    ""
       *   - \a safe_hostname    "[:]"
       *   - \a safe_pathname    "/"
       *   - \a safe_pathparams  ""
       *   - \a safe_querystr    ""
       *   - \a safe_fragment    ""
       *   .
       * .
       *
       * - Regular expressions used to verify URL components
       *   and their sub-components:
       *   - \a rx_scheme        "^[a-zA-Z][a-zA-Z0-9\\._-]*$"
       *   - \a rx_username      ".*"
       *   - \a rx_password      ".*"
       *   - \a rx_hostname      ".*" FIXME!
       *   - \a rx_port          "^[0-9]{1,5}$"
       *   - \a rx_pathname      ".*"
       *   - \a rx_pathparams    ".*"
       *   - \a rx_querystr      ".*"
       *   - \a rx_fragment      ".*"
       *   .
       * .
       */
      virtual void
      configure();


      /**
       * Get the value of a UrlBase configuration variable.
       *
       * See configure() method for names an purpose of the
       * configuration variables used in UrlBase class.
       *
       * \param opt The name of the configuration variable.
       * \return The value of the specified variable
       *         or empty string.
       */
      std::string
      config(const std::string &opt) const;

    //protected:
      // friend class Url;
      /**
       * Set the value of a UrlBase configuration variable.
       *
       * See configure() method for names an purpose of the
       * configuration variables used in UrlBase class.
       *
       * \param opt The name of the configuration variable.
       * \param val The new value for the configuration variable.
       */
      void
      config(const std::string &opt, const std::string &val);


      /**
       * Return the view options of the current object.
       *
       * This method is used to query the view options
       * used by the Url::toString() method.
       *
       * \return The current view option combination.
       */
      ViewOptions
      getViewOptions() const;

      /**
       * Change the view options of the current object.
       *
       * This method is used to change the view options
       * used by the Url::toString() method.
       *
       * \param vopts New view options combination.
       */
      void
      setViewOptions(const ViewOptions &vopts);


    protected:
      /**
       * Utility method to cleanup a path name.
       *
       * By default, this method replaces multiple occurences
       * of the "/" characeter from the begining of the path,
       * so the cleaned path begins with at most one "/".
       *
       * This operation is required in some cases to not to
       * missinterpret multiple "/" occurences as an empty
       * URL authority.
       *
       * \param path A path name.
       * \return A path begining with at most one "/" character.
       */
      virtual std::string
      cleanupPathName(const std::string &path);


    private:
      UrlData        *m_data;
    };


    // ---------------------------------------------------------------
    /**
     * \brief Copy-On-Write Url reference.
     */
    typedef RWCOW_pointer<UrlBase>          UrlRef;


    //////////////////////////////////////////////////////////////////
  } // namespace url
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_URL_URLBASE_H */
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
