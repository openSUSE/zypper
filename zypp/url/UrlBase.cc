/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/url/UrlBase.cc
 */
#include <zypp/url/UrlBase.h>
#include <zypp/base/String.h>

#include <stdexcept>
// FIXME:
#if defined(WITH_URL_PARSE_DEBUG)
#include <iostream>
#endif

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    const ViewOptions ViewOptions::WITH_SCHEME       = 0x0001;
    const ViewOptions ViewOptions::WITH_USERNAME     = 0x0002;
    const ViewOptions ViewOptions::WITH_PASSWORD     = 0x0004;
    const ViewOptions ViewOptions::WITH_HOST         = 0x0008;
    const ViewOptions ViewOptions::WITH_PORT         = 0x0010;
    const ViewOptions ViewOptions::WITH_PATH_NAME    = 0x0020;
    const ViewOptions ViewOptions::WITH_PATH_PARAMS  = 0x0040;
    const ViewOptions ViewOptions::WITH_QUERY_STR    = 0x0080;
    const ViewOptions ViewOptions::WITH_FRAGMENT     = 0x0100;
    const ViewOptions ViewOptions::EMPTY_AUTHORITY   = 0x0200;
    const ViewOptions ViewOptions::EMPTY_PATH_NAME   = 0x0400;
    const ViewOptions ViewOptions::EMPTY_PATH_PARAMS = 0x0800;
    const ViewOptions ViewOptions::EMPTY_QUERY_STR   = 0x1000;
    const ViewOptions ViewOptions::EMPTY_FRAGMENT    = 0x2000;
    const ViewOptions ViewOptions::DEFAULTS          = //0x07bb;
                      ViewOptions::WITH_SCHEME       +
                      ViewOptions::WITH_USERNAME     +
                      ViewOptions::WITH_HOST         +
                      ViewOptions::WITH_PORT         +
                      ViewOptions::WITH_PATH_NAME    +
                      ViewOptions::WITH_QUERY_STR    +
                      ViewOptions::WITH_FRAGMENT     +
                      ViewOptions::EMPTY_AUTHORITY   +
                      ViewOptions::EMPTY_PATH_NAME;


    // ---------------------------------------------------------------
    /**
     * authority = //[user [:password] @ ] host [:port]
     *
     * host      = hostname | IPv4 | "[" IPv6-IP "]" | "[v...]"
     */
    #define RX_SPLIT_AUTHORITY \
    "^(([^:@]*)" "([:]([^@]*))?" "@)?" "(\\[[^]]+\\]|[^:]+)?" "([:](.*))?"


    // ---------------------------------------------------------------
    namespace // anonymous
    {

			// -------------------------------------------------------------
      inline void
      checkUrlData(const std::string &data,
             const std::string &name,
             const std::string &regx)
      {
        if( regx.empty() || regx == "^$")
        {
          throw std::invalid_argument(
            std::string("Url scheme does not allow a " +
                        name)
          );
        }
        else
        if( !str::regex_match(data, str::regex(regx)))
        {
          throw std::invalid_argument(
            std::string("Invalid " + name + " argument '" +
                        data + "'")
            );
        }
      }

    } // namespace


    // ---------------------------------------------------------------
    UrlBase::~UrlBase()
    {
      delete m_data;
      m_data = NULL;
    }


    // ---------------------------------------------------------------
    UrlBase::UrlBase()
      : m_data( new UrlData())
    {
      configure();
    }


    // ---------------------------------------------------------------
    UrlBase::UrlBase(const UrlBase &url)
      : m_data( new UrlData( *(url.m_data)))
    {
    }


    // ---------------------------------------------------------------
    UrlBase::UrlBase(const std::string &scheme,
                     const std::string &authority,
                     const std::string &pathdata,
                     const std::string &querystr,
                     const std::string &fragment)
      : m_data( new UrlData())
    {
      configure();
      init(scheme, authority, pathdata, querystr, fragment);
    }


    // ---------------------------------------------------------------
    void
    UrlBase::init(const std::string &scheme,
                  const std::string &authority,
                  const std::string &pathdata,
                  const std::string &querystr,
                  const std::string &fragment)
    {
      setScheme(scheme);
      setAuthority(authority);
      setPathData(pathdata);
      setQueryString(querystr);
      setFragment(fragment);
    }


    // ---------------------------------------------------------------
    void
    UrlBase::configure()
    {
      config("sep_pathparams",  ";");
      config("psep_pathparam",  ",");
      config("vsep_pathparam",  "=");

      config("psep_querystr",   "&");
      config("vsep_querystr",   "=");

      config("safe_username",   "");
      config("safe_password",   "");
      config("safe_hostname",   "[:]");
      config("safe_pathname",   "/");
      config("safe_pathparams", "");
      config("safe_querystr",   "");
      config("safe_fragment",   "");

      config("rx_scheme",       "^[a-zA-Z][a-zA-Z0-9\\._-]*$");
      config("rx_username",     ".*");
      config("rx_password",     ".*");
      config("rx_hostname",     ".*"); // FIXME
      config("rx_port",         ".*"); // FIXME
      config("rx_pathname",     ".*");
      config("rx_pathparams",   ".*");
      config("rx_querystr",     ".*");
      config("rx_fragment",     ".*");
    }


    // ---------------------------------------------------------------
    void
    UrlBase::config(const std::string &opt, const std::string &val)
    {
      m_data->config[opt] = val;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::config(const std::string &opt) const
    {
      UrlConfig::const_iterator v( m_data->config.find(opt));
      if( v != m_data->config.end())
        return v->second;
      else
        return std::string();
    }


    // ---------------------------------------------------------------
    ViewOptions
    UrlBase::getViewOptions() const
    {
      return m_data->vopts;
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setViewOptions(const ViewOptions &vopts)
    {
        m_data->vopts = vopts;
    }


    // ---------------------------------------------------------------
    void
    UrlBase::clear()
    {
      zypp::url::UrlConfig   config(m_data->config);
      zypp::url::ViewOptions vopts(m_data->vopts);
      *m_data = UrlData();
      m_data->config = config;
      m_data->vopts  = vopts;
    }


    // ---------------------------------------------------------------
    UrlBase *
    UrlBase::clone() const
    {
      return new UrlBase(*this);
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::cleanupPathName(const std::string &path)
    {
      size_t pos = 0;

      while( pos < path.length() && path.at(pos) == '/')
        pos++;

      if( pos > 1)
      {
        // make sure, there is not more than
        // _one_ leading "/" in the path name.
        return path.substr(pos - 1);
      }

      return std::string(path);
    }


    // ---------------------------------------------------------------
    UrlBase::Schemes
    UrlBase::getKnownSchemes() const
    {
      return Schemes();
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isKnownScheme(const std::string &scheme) const
    {
      std::string             lscheme( str::toLower(scheme));
      Schemes                 schemes( getKnownSchemes());
      Schemes::const_iterator s;

      for(s=schemes.begin(); s!=schemes.end(); ++s)
      {
        if( lscheme == str::toLower(*s))
          return true;
      }
      return false;
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isValidScheme(const std::string &scheme) const
    {
      if(scheme.empty() ||
         str::regex_match(scheme, str::regex(config("rx_scheme"))))
      {
        std::string lscheme( str::toLower(scheme));
        Schemes     schemes( getKnownSchemes());

        if( schemes.empty())
          return true;

        Schemes::const_iterator s;
        for(s=schemes.begin(); s!=schemes.end(); ++s)
        {
          if( lscheme == str::toLower(*s))
            return true;
        }
      }
      return false;
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isValid() const
    {
      return !getScheme().empty();
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::toString() const
    {
      return toString(getViewOptions());
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::toString(const zypp::url::ViewOptions &opts) const
    {
      std::string url;
      UrlData     tmp;

      if( opts.has(ViewOptions::WITH_SCHEME))
      {
        tmp.scheme = getScheme();
        if( !tmp.scheme.empty())
        {
          url += tmp.scheme + ":";

          if( opts.has(ViewOptions::WITH_HOST))
          {
            tmp.host = getHost(zypp::url::E_ENCODED);
            if( !tmp.host.empty())
            {
              url += "//";

              if( opts.has(ViewOptions::WITH_USERNAME))
              {
                tmp.user = getUsername(zypp::url::E_ENCODED);
                if( !tmp.user.empty())
                {
                  url += tmp.user;

                  if( opts.has(ViewOptions::WITH_PASSWORD))
                  {
                    tmp.pass = getPassword(zypp::url::E_ENCODED);
                    if( !tmp.pass.empty())
                    {
                      url += ":" + tmp.pass;
                    }
                  }
                  url += "@";
                }
              }

              url += tmp.host;

              if( opts.has(ViewOptions::WITH_PORT))
              {
                tmp.port = getPort();
                if( !tmp.port.empty())
                {
                  url += ":" + tmp.port;
                }
              }
            }
          }
          else if( opts.has(ViewOptions::EMPTY_AUTHORITY))
          {
            url += "//";
          }
        }
      }

      if( opts.has(ViewOptions::WITH_PATH_NAME))
      {
        tmp.pathname = getPathName(zypp::url::E_ENCODED);
        if( !tmp.pathname.empty())
        {
          if( (!tmp.host.empty() || opts.has(ViewOptions::EMPTY_AUTHORITY))
              && (tmp.pathname.at(0) != '/'))
          {
            url += "/";
          }
          url += tmp.pathname;

          if( opts.has(ViewOptions::WITH_PATH_PARAMS))
          {
            tmp.pathparams = getPathParams();
            if( !tmp.pathparams.empty())
            {
              url += ";" + tmp.pathparams;
            }
            else if( opts.has(ViewOptions::EMPTY_PATH_PARAMS))
            {
              url += ";";
            }
          }
        }
        else if( opts.has(ViewOptions::EMPTY_PATH_NAME))
        {
          url += "/";
          if( opts.has(ViewOptions::EMPTY_PATH_PARAMS))
          {
            url += ";";
          }
        }
      }

      if( opts.has(ViewOptions::WITH_QUERY_STR))
      {
        tmp.querystr = getQueryString();
        if( !tmp.querystr.empty())
        {
          url += "?" + tmp.querystr;
        }
        else if( opts.has(ViewOptions::EMPTY_QUERY_STR))
        {
          url += "?";
        }
      }

      if( opts.has(ViewOptions::WITH_FRAGMENT))
      {
        tmp.fragment = getFragment(zypp::url::E_ENCODED);
        if( !tmp.fragment.empty())
        {
          url += "#" + tmp.fragment;
        }
        else if( opts.has(ViewOptions::EMPTY_FRAGMENT))
        {
          url += "#";
        }
      }

      return url;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getScheme() const
    {
      return m_data->scheme;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getAuthority() const
    {
      std::string str;
      if( !getHost(zypp::url::E_ENCODED).empty())
      {
        if( !getUsername(zypp::url::E_ENCODED).empty())
        {
          str = getUsername(zypp::url::E_ENCODED);
          if( !getPassword(zypp::url::E_ENCODED).empty())
          {
            str += ":" + getPassword(zypp::url::E_ENCODED);
          }
          str += "@";
        }

        str += getHost(zypp::url::E_ENCODED);
        if( !getPort().empty())
        {
          str += ":" + getPort();
        }
      }
      return str;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPathData() const
    {
      return getPathName(zypp::url::E_ENCODED) +
             config("sep_pathparams") +
             getPathParams();
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getQueryString() const
    {
      return m_data->querystr;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getFragment(EEncoding eflag) const
    {
      if(eflag == zypp::url::E_DECODED)
        return zypp::url::decode(m_data->fragment);
      else
        return m_data->fragment;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getUsername(EEncoding eflag) const
    {
      if(eflag == zypp::url::E_DECODED)
        return zypp::url::decode(m_data->user);
      else
        return m_data->user;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPassword(EEncoding eflag) const
    {
      if(eflag == zypp::url::E_DECODED)
        return zypp::url::decode(m_data->pass);
      else
        return m_data->pass;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getHost(EEncoding eflag) const
    {
      if(eflag == zypp::url::E_DECODED)
        return zypp::url::decode(m_data->host);
      else
        return m_data->host;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPort() const
    {
      return m_data->port;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPathName(EEncoding eflag) const
    {
      if(eflag == zypp::url::E_DECODED)
        return zypp::url::decode(m_data->pathname);
      else
        return m_data->pathname;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPathParams() const
    {
      return m_data->pathparams;
    }


    // ---------------------------------------------------------------
    zypp::url::ParamVec
    UrlBase::getPathParamsVec() const
    {
      zypp::url::ParamVec pvec;
      zypp::url::split(
        pvec,
        getPathParams(),
        config("psep_pathparam")
      );
      return pvec;
    }


    // ---------------------------------------------------------------
    zypp::url::ParamMap
    UrlBase::getPathParamsMap(EEncoding eflag) const
    {
      zypp::url::ParamMap pmap;
      zypp::url::split(
        pmap,
        getPathParams(),
        config("psep_pathparam"),
        config("vsep_pathparam"),
        eflag
      );
      return pmap;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getPathParam(const std::string &param, EEncoding eflag) const
    {
      zypp::url::ParamMap pmap( getPathParamsMap( eflag));
      zypp::url::ParamMap::const_iterator i( pmap.find(param));

      return i != pmap.end() ? i->second : std::string();
    }


    // ---------------------------------------------------------------
    zypp::url::ParamVec
    UrlBase::getQueryStringVec() const
    {
      zypp::url::ParamVec pvec;
      zypp::url::split(
        pvec,
        getQueryString(),
        config("psep_querystr")
      );
      return pvec;
    }


    // ---------------------------------------------------------------
    zypp::url::ParamMap
    UrlBase::getQueryStringMap(EEncoding eflag) const
    {
      zypp::url::ParamMap pmap;
      zypp::url::split(
        pmap,
        getQueryString(),
        config("psep_querystr"),
        config("vsep_querystr"),
        eflag
      );
      return pmap;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::getQueryParam(const std::string &param, EEncoding eflag) const
    {
      zypp::url::ParamMap pmap( getQueryStringMap( eflag));
      zypp::url::ParamMap::const_iterator i( pmap.find(param));

      return i != pmap.end() ? i->second : std::string();
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setScheme(const std::string &scheme)
    {
      if( isValidScheme(scheme))
      {
        m_data->scheme = str::toLower(scheme);
      }
      else
      {
        throw std::invalid_argument(
          std::string("Invalid Url scheme '" + scheme + "'")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setAuthority(const std::string &authority)
    {
      str::regex  rex(RX_SPLIT_AUTHORITY);
      str::smatch out;
      bool        ret = str::regex_match(authority, out, rex);

      // FIXME:
      #if defined(WITH_URL_PARSE_DEBUG)
      std::cerr << "Regex parsed URL authority("
                << out.size() << "):" << std::endl;
      std::cerr << "==> " << authority << std::endl;
      for(size_t n=0; n < out.size(); n++)
      {
        std::cerr << "[" << n << "] " << out[n].str() << std::endl;
      }
      #endif

      if( ret && out.size() == 8)
      {
        setUsername(out[2].str());
        setPassword(out[4].str());
        setHost(out[5].str());
        setPort(out[7].str());
      }
      else
      {
        throw std::invalid_argument(
          "Unable to parse Url authority"
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathData(const std::string &pathdata)
    {
      size_t pos;
      pos = pathdata.find(config("sep_pathparams"));
      if( pos != std::string::npos)
      {
        setPathName(pathdata.substr(0, pos));
        setPathParams(pathdata.substr(pos + 1));
      }
      else
      {
        setPathName(pathdata);
        setPathParams("");
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setQueryString(const std::string &querystr)
    {
      if( querystr.empty())
      {
        m_data->querystr = querystr;
      }
      else
      {
        checkUrlData(querystr, "query string", config("rx_querystr"));

        // FIXME: split & recode?
        m_data->querystr = querystr;
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setFragment(const std::string &fragment)
    {
      if( fragment.empty())
      {
        m_data->fragment = fragment;
      }
      else
      {
        std::string data( zypp::url::decode(fragment));

        checkUrlData(data, "fragment", config("rx_fragment"));

        m_data->fragment = zypp::url::encode(
          data, config("safe_fragment")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setUsername(const std::string &user)
    {
      if( user.empty())
      {
        m_data->user = user;
      }
      else
      {
        std::string data( zypp::url::decode(user));

        checkUrlData(data, "username", config("rx_username"));

        m_data->user = zypp::url::encode(
          data, config("safe_username")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPassword(const std::string &pass)
    {
      if( pass.empty())
      {
        m_data->pass = pass;
      }
      else
      {
        std::string data( zypp::url::decode(pass));

        checkUrlData(data, "password", config("rx_password"));

        m_data->pass = zypp::url::encode(
          data, config("safe_password")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setHost(const std::string &host)
    {
      if( host.empty())
      {
        m_data->host = host;
      }
      else
      {
        std::string data( zypp::url::decode(host));

        checkUrlData(data, "hostname", config("rx_hostname"));

        m_data->host = zypp::url::encode(
          data, config("safe_hostname")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPort(const std::string &port)
    {
      if( port.empty())
      {
        m_data->port = port;
      }
      else
      {
        std::string data( zypp::url::decode(port));

        checkUrlData(data, "port", config("rx_port"));

        m_data->port = data;
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathName(const std::string &path)
    {
      if( path.empty())
      {
        m_data->pathname = path;
      }
      else
      {
        std::string data( cleanupPathName(zypp::url::decode(path)));

        checkUrlData(data, "path", config("rx_pathname"));

        m_data->pathname = zypp::url::encode(
          data, config("safe_pathname")
        );
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathParams(const std::string &params)
    {
      if( params.empty())
      {
        m_data->pathparams = params;
      }
      else
      {
        checkUrlData(params, "path parameters", config("rx_pathparams"));

        // FIXME: split & recode?
        m_data->pathparams = params;
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathParamsVec(const zypp::url::ParamVec &pvec)
    {
      setPathParams(
        zypp::url::join(
          pvec,
          config("psep_pathparam")
        )
      );
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathParamsMap(const zypp::url::ParamMap &pmap)
    {
      setPathParams(
        zypp::url::join(
          pmap,
          config("psep_pathparam"),
          config("vsep_pathparam"),
          config("safe_pathparams")
        )
      );
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathParam(const std::string &param, const std::string &value)
    {
          std::string raw_param( zypp::url::decode(param));
          std::string raw_value( zypp::url::decode(value));

          zypp::url::ParamMap pmap( getPathParamsMap(zypp::url::E_DECODED));
          pmap[raw_param] = raw_value;

          setPathParamsMap(pmap);
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setQueryStringVec(const zypp::url::ParamVec &pvec)
    {
      setQueryString(
        zypp::url::join(
          pvec,
          config("psep_querystr")
        )
      );
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setQueryStringMap(const zypp::url::ParamMap &pmap)
    {
      setQueryString(
        zypp::url::join(
          pmap,
          config("psep_querystr"),
          config("vsep_querystr"),
          config("safe_querystr")
        )
      );
    }

    // ---------------------------------------------------------------
    void
    UrlBase::setQueryParam(const std::string &param, const std::string &value)
    {
          std::string raw_param( zypp::url::decode(param));
          std::string raw_value( zypp::url::decode(value));

          zypp::url::ParamMap pmap( getQueryStringMap(zypp::url::E_DECODED));
          pmap[raw_param] = raw_value;

          setQueryStringMap(pmap);
    }


    //////////////////////////////////////////////////////////////////
  } // namespace url
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
