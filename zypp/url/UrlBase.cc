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
#include "zypp/url/UrlBase.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Regex.h"

#include <stdexcept>
#include <climits>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>

// in the Estonian locale, a-z excludes t, for example. #302525
// http://en.wikipedia.org/wiki/Estonian_alphabet
#define a_zA_Z "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

// ---------------------------------------------------------------
/*
** authority = //[user [:password] @ ] host [:port]
**
** host      = hostname | IPv4 | "[" IPv6-IP "]" | "[v...]"
*/
#define RX_VALID_SCHEME    "^[" a_zA_Z "][" a_zA_Z "0-9\\.+-]*$"

#define RX_VALID_PORT      "^[0-9]{1,5}$"

#define RX_VALID_HOSTNAME  "^[[:alnum:]]+([\\.-][[:alnum:]]+)*$"

#define RX_VALID_HOSTIPV4  \
        "^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})$"

#define RX_VALID_HOSTIPV6  \
        "^\\[[:a-fA-F0-9]+(:[0-9]{1,3}(\\.[0-9]{1,3}){3})?\\]$"


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    /*
    ** URL asString() view option constants:
    */
    const ViewOption  ViewOption::WITH_SCHEME       = 0x0001;
    const ViewOption  ViewOption::WITH_USERNAME     = 0x0002;
    const ViewOption  ViewOption::WITH_PASSWORD     = 0x0004;
    const ViewOption  ViewOption::WITH_HOST         = 0x0008;
    const ViewOption  ViewOption::WITH_PORT         = 0x0010;
    const ViewOption  ViewOption::WITH_PATH_NAME    = 0x0020;
    const ViewOption  ViewOption::WITH_PATH_PARAMS  = 0x0040;
    const ViewOption  ViewOption::WITH_QUERY_STR    = 0x0080;
    const ViewOption  ViewOption::WITH_FRAGMENT     = 0x0100;
    const ViewOption  ViewOption::EMPTY_AUTHORITY   = 0x0200;
    const ViewOption  ViewOption::EMPTY_PATH_NAME   = 0x0400;
    const ViewOption  ViewOption::EMPTY_PATH_PARAMS = 0x0800;
    const ViewOption  ViewOption::EMPTY_QUERY_STR   = 0x1000;
    const ViewOption  ViewOption::EMPTY_FRAGMENT    = 0x2000;
    const ViewOption  ViewOption::DEFAULTS          = 0x07bb;
    /*
    const ViewOption  ViewOption::DEFAULTS          =
                      ViewOption::WITH_SCHEME       +
                      ViewOption::WITH_USERNAME     +
                      ViewOption::WITH_HOST         +
                      ViewOption::WITH_PORT         +
                      ViewOption::WITH_PATH_NAME    +
                      ViewOption::WITH_QUERY_STR    +
                      ViewOption::WITH_FRAGMENT     +
                      ViewOption::EMPTY_AUTHORITY   +
                      ViewOption::EMPTY_PATH_NAME;
    */

    // ---------------------------------------------------------------
    ViewOption::ViewOption()
      : opt(0x07bb)
    {}

    // ---------------------------------------------------------------
    ViewOption::ViewOption(int option)
      : opt(option)
    {}


    // ---------------------------------------------------------------
    /*
    ** Behaviour configuration variables.
    */
    typedef std::map< std::string, std::string > UrlConfig;


    // ---------------------------------------------------------------
    /**
     * \brief Internal data used by UrlBase.
     */
    class UrlBaseData
    {
    public:
      UrlBaseData()
      {}

      UrlBaseData(const UrlConfig &conf)
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
    /*
    ** Anonymous/internal utility namespace:
    */
    namespace // anonymous
    {

			// -------------------------------------------------------------
      inline void
      checkUrlData(const std::string &data,
                   const std::string &name,
                   const std::string &regx,
                   bool               show=true)
      {
        if( regx.empty() || regx == "^$")
        {
          ZYPP_THROW(UrlNotAllowedException(
            str::form(_("Url scheme does not allow a %s"), name.c_str())
          ));
        }
        else
        {
          bool valid = false;
          try
          {
            str::regex rex(regx);
            valid = str::regex_match(data, rex);
          }
          catch( ... )
          {}

          if( !valid)
          {
            if( show)
            {
              ZYPP_THROW(UrlBadComponentException(
                str::form(_("Invalid %s component '%s'"),
                          name.c_str(), data.c_str())
              ));
            }
            else
            {
              ZYPP_THROW(UrlBadComponentException(
                str::form(_("Invalid %s component"), name.c_str())
              ));
            }
          }
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
      : m_data( new UrlBaseData())
    {
      configure();
    }


    // ---------------------------------------------------------------
    UrlBase::UrlBase(const UrlBase &url)
      : m_data( new UrlBaseData( *(url.m_data)))
    {
    }


    // ---------------------------------------------------------------
    UrlBase::UrlBase(const std::string &scheme,
                     const std::string &authority,
                     const std::string &pathdata,
                     const std::string &querystr,
                     const std::string &fragment)
      : m_data( new UrlBaseData())
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
      if ( scheme.empty() && *pathdata.c_str() == '/' )
	setScheme("file");
      else
	setScheme(scheme);

      setAuthority(authority);
      setPathData(pathdata);
      setQueryString(querystr);
      setFragment(fragment, zypp::url::E_ENCODED);
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

      config("safe_username",   "~!$&'()*+=,;");
      config("safe_password",   "~!$&'()*+=,:;");
      config("safe_hostname",   "[:]");
      config("safe_pathname",   "~!$&'()*+=,:@/");
      config("safe_pathparams", "~!$&'()*+=,:;@/");
      config("safe_querystr",   "~!$&'()*+=,:;@/?");
      config("safe_fragment",   "~!$&'()*+=,:;@/?");

      // y=yes (allowed)
      // n=no  (disallowed, exception if !empty)
      config("with_authority",  "y");
      config("with_port",       "y");

      // y=yes (required but don't throw if empty)
      // n=no  (not required, ignore if empty)
      // m=mandatory (exception if empty)
      config("require_host",    "n");
      config("require_pathname","n");

      // y=yes (encode 2. slash even if authority present)
      // n=no  (don't encode 2. slash if authority present)
      config("path_encode_slash2", "n");

      config("rx_username",     "^([" a_zA_Z "0-9!$&'\\(\\)*+=,;~\\._-]|%[a-fA-F0-9]{2})+$");
      config("rx_password",     "^([" a_zA_Z "0-9!$&'\\(\\)*+=,:;~\\._-]|%[a-fA-F0-9]{2})+$");

      config("rx_pathname",     "^([" a_zA_Z "0-9!$&'\\(\\){}*+=,:@/~\\._-]|%[a-fA-F0-9]{2})+$");
      config("rx_pathparams",   "^([" a_zA_Z "0-9!$&'\\(\\){}*+=,:;@/~\\._-]|%[a-fA-F0-9]{2})+$");

      config("rx_querystr",     "^([" a_zA_Z "0-9!$&'\\(\\){}*+=,:;@/?~\\._-]|%[a-fA-F0-9]{2})+$");
      config("rx_fragment",     "^([" a_zA_Z "0-9!$&'\\(\\){}*+=,:;@/?~\\._-]|%[a-fA-F0-9]{2})+$");
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
      *m_data = UrlBaseData();
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
    zypp::url::UrlSchemes
    UrlBase::getKnownSchemes() const
    {
      return UrlSchemes();
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isKnownScheme(const std::string &scheme) const
    {
      std::string                lscheme( str::toLower(scheme));
      UrlSchemes                 schemes( getKnownSchemes());
      UrlSchemes::const_iterator s;

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
      bool valid = false;
      try
      {
        str::regex rex(RX_VALID_SCHEME);
        valid = str::regex_match(scheme, rex);
      }
      catch( ... )
      {}

      if(valid)
      {
        std::string    lscheme( str::toLower(scheme));
        UrlSchemes     schemes( getKnownSchemes());

        if( schemes.empty())
          return true;

        UrlSchemes::const_iterator s;
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
      /*
      ** scheme is the only mandatory component
      ** for all url's and is already verified,
      ** (except for empty Url instances), so
      ** Url with empty scheme is never valid.
      */
      if( getScheme().empty())
        return false;

      std::string host( getHost(zypp::url::E_ENCODED));
      if( host.empty() && config("require_host")     != "n")
        return false;

      std::string path( getPathName(zypp::url::E_ENCODED));
      if( path.empty() && config("require_pathname") != "n")
        return false;

      /*
      ** path has to begin with "/" if authority avaliable
      ** if host is set after the pathname, we can't throw
      */
      if( !host.empty() && !path.empty() && path.at(0) != '/')
        return false;

      return true;
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::asString() const
    {
      return asString(getViewOptions());
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::asString(const zypp::url::ViewOptions &opts) const
    {
      std::string   url;
      UrlBaseData   tmp;

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
            else if( opts.has(ViewOptions::EMPTY_AUTHORITY))
            {
              url += "//";
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
          if(url.find("/") != std::string::npos)
          {
            // Url contains authority (that may be empty),
            // we may need a rewrite of the encoded path.
            tmp.pathname = cleanupPathName(tmp.pathname, true);
            if(tmp.pathname.at(0) != '/')
            {
              url += "/";
            }
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
        else if( opts.has(ViewOptions::EMPTY_PATH_NAME)
                 && url.find("/") != std::string::npos)
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
        return cleanupPathName(m_data->pathname);
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
      if( config("psep_pathparam").empty())
      {
        pvec.push_back(getPathParams());
      }
      else
      {
        zypp::url::split(
          pvec,
          getPathParams(),
          config("psep_pathparam")
        );
      }
      return pvec;
    }


    // ---------------------------------------------------------------
    zypp::url::ParamMap
    UrlBase::getPathParamsMap(EEncoding eflag) const
    {
      if( config("psep_pathparam").empty() ||
          config("vsep_pathparam").empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          "Path parameter parsing not supported for this URL"
        ));
      }
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
      if( config("psep_querystr").empty())
      {
        pvec.push_back(getQueryString());
      }
      else
      {
        zypp::url::split(
          pvec,
          getQueryString(),
          config("psep_querystr")
        );
      }
      return pvec;
    }


    // ---------------------------------------------------------------
    zypp::url::ParamMap
    UrlBase::getQueryStringMap(EEncoding eflag) const
    {
      if( config("psep_querystr").empty() ||
          config("vsep_querystr").empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          _("Query string parsing not supported for this URL")
        ));
      }
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
      if( scheme.empty())
      {
        ZYPP_THROW(UrlBadComponentException(
          _("Url scheme is a required component")
        ));
      }
      else
      {
        ZYPP_THROW(UrlBadComponentException(
          str::form(_("Invalid Url scheme '%s'"), scheme.c_str())
        ));
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setAuthority(const std::string &authority)
    {
      std::string s = authority;
      std::string::size_type p,q;

      std::string username, password, host, port;

      if ((p=s.find('@')) != std::string::npos)
      {
        q = s.find(':');
        if (q != std::string::npos && q < p)
        {
          setUsername(s.substr(0, q), zypp::url::E_ENCODED);
          setPassword(s.substr(q+1, p-q-1), zypp::url::E_ENCODED);
        }
        else
          setUsername(s.substr(0, p), zypp::url::E_ENCODED);
        s = s.substr(p+1);
      }
      if ((p = s.rfind(':')) != std::string::npos && ( (q = s.rfind(']')) == std::string::npos || q < p) )
      {
        setHost(s.substr(0, p));
        setPort(s.substr(p+1));
      }
      else
        setHost(s);
    }

    // ---------------------------------------------------------------
    void
    UrlBase::setPathData(const std::string &pathdata)
    {
      size_t      pos = std::string::npos;
      std::string sep(config("sep_pathparams"));

      if( !sep.empty())
        pos = pathdata.find(sep);

      if( pos != std::string::npos)
      {
        setPathName(pathdata.substr(0, pos),
                    zypp::url::E_ENCODED);
        setPathParams(pathdata.substr(pos + 1));
      }
      else
      {
        setPathName(pathdata,
                    zypp::url::E_ENCODED);
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

        m_data->querystr = querystr;
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setFragment(const std::string &fragment,
                         EEncoding         eflag)
    {
      if( fragment.empty())
      {
        m_data->fragment = fragment;
      }
      else
      {
        if(eflag == zypp::url::E_ENCODED)
        {
          checkUrlData(fragment, "fragment", config("rx_fragment"));

          m_data->fragment = fragment;
        }
        else
        {
          m_data->fragment = zypp::url::encode(
            fragment, config("safe_fragment")
          );
        }
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setUsername(const std::string &user,
                         EEncoding         eflag)
    {
      if( user.empty())
      {
        m_data->user = user;
      }
      else
      {
        if( config("with_authority") != "y")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme does not allow a username")
          ));
        }

        if(eflag == zypp::url::E_ENCODED)
        {
          checkUrlData(user, "username", config("rx_username"));

          m_data->user = user;
        }
        else
        {
          m_data->user = zypp::url::encode(
            user, config("safe_username")
          );
        }
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPassword(const std::string &pass,
                         EEncoding         eflag)
    {
      if( pass.empty())
      {
        m_data->pass = pass;
      }
      else
      {
        if( config("with_authority") != "y")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme does not allow a password")
          ));
        }

        if(eflag == zypp::url::E_ENCODED)
        {
          checkUrlData(pass, "password", config("rx_password"), false);

          m_data->pass = pass;
        }
        else
        {
          m_data->pass = zypp::url::encode(
            pass, config("safe_password")
          );
        }
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setHost(const std::string &host)
    {
      if( host.empty())
      {
        if(config("require_host") == "m")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme requires a host component")
          ));
        }
        m_data->host = host;
      }
      else
      {
        if( config("with_authority") != "y")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme does not allow a host component")
          ));
        }

        if( isValidHost(host))
        {
          std::string temp;

          // always decode in case isValidHost()
          // is reimplemented and supports also
          // the [v ... ] notation.
          if( host.at(0) == '[')
          {
            temp = str::toUpper(zypp::url::decode(host));
          }
          else
          {
            temp = str::toLower(zypp::url::decode(host));
          }

          m_data->host = zypp::url::encode(
            temp, config("safe_hostname")
          );
        }
        else
        {
          ZYPP_THROW(UrlBadComponentException(
            str::form(_("Invalid host component '%s'"), host.c_str())
          ));
        }
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
        if( config("with_authority") != "y" ||
            config("with_port")      != "y")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme does not allow a port")
          ));
        }

        if( isValidPort(port))
        {
          m_data->port = port;
        }
        else
        {
          ZYPP_THROW(UrlBadComponentException(
            str::form(_("Invalid port component '%s'"), port.c_str())
          ));
        }
      }
    }


    // ---------------------------------------------------------------
    void
    UrlBase::setPathName(const std::string &path,
                         EEncoding         eflag)
    {
      if( path.empty())
      {
        if(config("require_pathname") == "m")
        {
          ZYPP_THROW(UrlNotAllowedException(
            _("Url scheme requires path name")
          ));
        }
        m_data->pathname = path;
      }
      else
      {
        if(eflag == zypp::url::E_ENCODED)
        {
          checkUrlData(path, "path name", config("rx_pathname"));

          if( !getHost(zypp::url::E_ENCODED).empty())
          {
            // has to begin with a "/". For consistency with
            // setPathName while the host is empty, we allow
            // it in encoded ("%2f") form - cleanupPathName()
            // will fix / decode the first slash if needed.
            if(!(path.at(0) == '/' || (path.size() >= 3 &&
                 str::toLower(path.substr(0, 3)) == "%2f")))
            {
              ZYPP_THROW(UrlNotAllowedException(
                _("Relative path not allowed if authority exists")
              ));
            }
          }

          m_data->pathname = cleanupPathName(path);
        }
        else //     zypp::url::E_DECODED
        {
          if( !getHost(zypp::url::E_ENCODED).empty())
          {
            if(path.at(0) != '/')
            {
              ZYPP_THROW(UrlNotAllowedException(
                _("Relative path not allowed if authority exists")
              ));
            }
          }

          m_data->pathname = cleanupPathName(
            zypp::url::encode(
              path, config("safe_pathname")
            )
          );
        }
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
      if( config("psep_pathparam").empty() ||
          config("vsep_pathparam").empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          "Path Parameter parsing not supported for this URL"
        ));
      }
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
          zypp::url::ParamMap pmap( getPathParamsMap(zypp::url::E_DECODED));
          pmap[param] = value;
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
      if( config("psep_querystr").empty() ||
          config("vsep_querystr").empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          _("Query string parsing not supported for this URL")
        ));
      }
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
          zypp::url::ParamMap pmap( getQueryStringMap(zypp::url::E_DECODED));
          pmap[param] = value;
          setQueryStringMap(pmap);
    }

    // ---------------------------------------------------------------
    void
    UrlBase::delQueryParam(const std::string &param)
    {
          zypp::url::ParamMap pmap( getQueryStringMap(zypp::url::E_DECODED));
          pmap.erase(param);
          setQueryStringMap(pmap);
    }


    // ---------------------------------------------------------------
    std::string
    UrlBase::cleanupPathName(const std::string &path) const
    {
      bool authority = !getHost(zypp::url::E_ENCODED).empty();
      return cleanupPathName(path, authority);
    }

    // ---------------------------------------------------------------
    std::string
    UrlBase::cleanupPathName(const std::string &path, bool authority) const
    {
      std::string copy( path);

      // decode the first slash if it is encoded ...
      if(copy.size() >= 3 && copy.at(0) != '/' &&
         str::toLower(copy.substr(0, 3)) == "%2f")
      {
        copy.replace(0, 3, "/");
      }

      // if path begins with a double slash ("//"); encode the second
      // slash [minimal and IMO sufficient] before the first path
      // segment, to fulfill the path-absolute rule of RFC 3986
      // disallowing a "//" if no authority is present.
      if( authority)
      {
        //
        // rewrite of "//" to "/%2f" not required, use config
        //
        if(config("path_encode_slash2") == "y")
        {
          // rewrite "//" ==> "/%2f"
          if(copy.size() >= 2 && copy.at(0) == '/' && copy.at(1) == '/')
          {
            copy.replace(1, 1, "%2F");
          }
        }
        else
        {
          // rewrite "/%2f" ==> "//"
          if(copy.size() >= 4 && copy.at(0) == '/' &&
             str::toLower(copy.substr(1, 4)) == "%2f")
          {
            copy.replace(1, 4, "/");
          }
        }
      }
      else
      {
        // rewrite of "//" to "/%2f" is required (no authority)
        if(copy.size() >= 2 && copy.at(0) == '/' && copy.at(1) == '/')
        {
          copy.replace(1, 1, "%2F");
        }
      }
      return copy;
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isValidHost(const std::string &host) const
    {
      try
      {
        str::regex regx(RX_VALID_HOSTIPV6);
        if( str::regex_match(host, regx))
        {
          struct in6_addr ip;
          std::string temp( host.substr(1, host.size()-2));

          return inet_pton(AF_INET6, temp.c_str(), &ip) > 0;
        }
        else
        {
          // matches also IPv4 dotted-decimal adresses...
          std::string temp( zypp::url::decode(host));
          str::regex  regx(RX_VALID_HOSTNAME);
          return str::regex_match(temp, regx);
        }
      }
      catch( ... )
      {}

      return false;
    }


    // ---------------------------------------------------------------
    bool
    UrlBase::isValidPort(const std::string &port) const
    {
      try
      {
        str::regex regx(RX_VALID_PORT);
        if( str::regex_match(port, regx))
        {
          long pnum = str::strtonum<long>(port);
          return ( pnum >= 1 && pnum <= USHRT_MAX);
        }
      }
      catch( ... )
      {}
      return false;
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
