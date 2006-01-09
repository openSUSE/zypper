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
    /** Url behaviour configuration container.
     */
    typedef std::map< std::string, std::string > UrlConfig;


    // ---------------------------------------------------------------
    /** Url toString() view options.
     */
    struct ViewOption
    {
      /**
        * @{
        * Flags 
        */
      static const ViewOption WITH_SCHEME;
      static const ViewOption WITH_USERNAME;
      static const ViewOption WITH_PASSWORD;
      static const ViewOption WITH_HOST;
      static const ViewOption WITH_PORT;
      static const ViewOption WITH_PATH_NAME;
      static const ViewOption WITH_PATH_PARAMS;
      static const ViewOption WITH_QUERY_STR;
      static const ViewOption WITH_FRAGMENT;
      /* @} */

      /**
       * @{
       * With Empty flags.
       */
      static const ViewOption EMPTY_AUTHORITY;
      static const ViewOption EMPTY_PATH_NAME;
      static const ViewOption EMPTY_PATH_PARAMS;
      static const ViewOption EMPTY_QUERY_STR;
      static const ViewOption EMPTY_FRAGMENT;
      /* @} */

      /** Default combination of view options.
       */
      static const ViewOption DEFAULTS;

      ViewOption(): opt(DEFAULTS.opt)
      {}

      /** @return The result of a add of \p r from \p l.
       */
      friend inline ViewOption
      operator + (const ViewOption &l, const ViewOption &r)
      { return ViewOption(l.opt |  r.opt); }

      /** @return The result of a subtract of \p r from \p l.
       */
      friend inline ViewOption
      operator - (const ViewOption &l, const ViewOption &r)
      { return ViewOption(l.opt & ~r.opt); }

      /** Check if option is set.
       * \param o    A view option.
       * \return True, if the current options bitwise matches
       *         the specified one.
       */
      inline bool
      has(const ViewOption &o) const
      { return o.opt & opt; }

    private:
      ViewOption(int o): opt(o) {}
      int opt;
    };


    // ---------------------------------------------------------------
    typedef ViewOption ViewOptions;


    // ---------------------------------------------------------------
    /**
     * FIXME:
     */
    class UrlData
    {
    public:
      /**
       * FIXME:
       */
      UrlData()
      {}

      /**
       * FIXME:
       */
      UrlData(const UrlConfig &conf)
        : config(conf)
      {}

      /**
       * FIXME:
       */
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
     * FIXME:
     */
    class UrlBase
    {
    public:
      /**
       * FIXME:
       */
      typedef std::vector<std::string>        Schemes;

      /**
       * FIXME:
       */
      UrlBase();

      /**
       * FIXME:
       */
      UrlBase(const UrlBase &url);

      /**
       * FIXME:
       */
      UrlBase(const std::string &scheme,
              const std::string &authority,
              const std::string &pathdata,
              const std::string &querystr,
              const std::string &fragment);

      /**
       * FIXME:
       */
      virtual ~UrlBase();

      /**
       * FIXME:
       */
      virtual void
      clear();

      /**
       * FIXME:
       */
      virtual UrlBase *
      clone() const;

      /**
       * FIXME:
       */
      virtual void
      init(const std::string &scheme,
           const std::string &authority,
           const std::string &pathdata,
           const std::string &querystr,
           const std::string &fragment);

      /**
       * FIXME:
       */
      virtual UrlBase::Schemes
      getKnownSchemes() const;

      /**
       * FIXME:
       */
      virtual bool
      isKnownScheme(const std::string &scheme) const;

      /**
       * FIXME:
       */
      virtual bool
      isValidScheme(const std::string &scheme) const;


      /**
       * FIXME:
       */
      virtual bool
      isValid() const;


      /**
       * FIXME:
       * TODO: hide pass, ...
       */
      virtual std::string
      toString() const;

      virtual std::string
      toString(const zypp::url::ViewOptions &opts) const;


      // -----------------
      virtual std::string
      getScheme() const;

      virtual std::string
      getAuthority() const;

      virtual std::string
      getPathData() const;

      virtual std::string
      getQueryString() const;

      virtual std::string
      getFragment(EEncoding eflag) const;


      // -----------------
      virtual std::string
      getUsername(EEncoding eflag) const;

      virtual std::string
      getPassword(EEncoding eflag) const;

      virtual std::string
      getHost(EEncoding eflag) const;

      virtual std::string
      getPort() const;

      virtual std::string
      getPathName(EEncoding eflag) const;

      virtual std::string
      getPathParams() const;


      // -----------------
      virtual zypp::url::ParamVec
      getPathParamsVec() const;

      virtual zypp::url::ParamMap
      getPathParamsMap(EEncoding eflag) const;

      virtual std::string
      getPathParam(const std::string &param, EEncoding eflag) const;


      virtual zypp::url::ParamVec
      getQueryStringVec() const;

      virtual zypp::url::ParamMap
      getQueryStringMap(EEncoding eflag) const;

      virtual std::string
      getQueryParam(const std::string &param, EEncoding eflag) const;


      // -----------------
      virtual void
      setScheme(const std::string &scheme);

      virtual void
      setAuthority(const std::string &authority);

      virtual void
      setPathData(const std::string &pathdata);

      virtual void
      setQueryString(const std::string &querystr);

      virtual void
      setFragment(const std::string &fragment);


      // -----------------
      virtual void
      setUsername(const std::string &user);

      virtual void
      setPassword(const std::string &pass);

      virtual void
      setHost(const std::string &host);

      virtual void
      setPort(const std::string &port);

      virtual void
      setPathName(const std::string &path);

      virtual void
      setPathParams(const std::string &params);


      // -----------------
      virtual void
      setPathParamsVec(const zypp::url::ParamVec &pvec);

      virtual void
      setPathParamsMap(const zypp::url::ParamMap &pmap);

      virtual void
      setPathParam(const std::string &param, const std::string &value);

      virtual void
      setQueryStringVec(const zypp::url::ParamVec &pmap);

      virtual void
      setQueryStringMap(const zypp::url::ParamMap &pmap);

      virtual void
      setQueryParam(const std::string &param, const std::string &value);


      // -----------------
      virtual void
      configure();

      std::string
      config(const std::string &opt) const;

    //protected:
      // friend class Url;
      void
      config(const std::string &opt, const std::string &val);

      ViewOptions
      getViewOptions() const;

      void
      setViewOptions(const ViewOptions &vopts);

    private:
      UrlData        *m_data;
    };


    // ---------------------------------------------------------------
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
