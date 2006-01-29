/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/Url.cc
 */
#include <zypp/Url.h>
#include <zypp/base/String.h>
#include <stdexcept>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////


  using namespace zypp::url;


  // -----------------------------------------------------------------
  /*
   * url       = [scheme:] [//authority] /path [?query] [#fragment]
   */
  #define RX_SPLIT_URL                       "^(([^:/?#]+):)?" \
                                             "(//([^/?#]*))?"  \
                                             "([^?#]*)"        \
                                             "([?]([^#]*))?"   \
                                             "(#(.*))?"


  ////////////////////////////////////////////////////////////////////
  namespace
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    class LDAPUrl: public UrlBase
    {
    public:
      LDAPUrl(): UrlBase()
      {
        configure();
      }

      LDAPUrl(const LDAPUrl &url): UrlBase(url)
      {}

      virtual UrlBase *
      clone() const
      {
        return new LDAPUrl(*this);
      }

      virtual UrlSchemes
      getKnownSchemes() const
      {
        UrlSchemes schemes(2);
        schemes[0] = "ldap";
        schemes[1] = "ldaps";
        return schemes;
      }

      virtual void
      configure()
      {
        config("sep_pathparams",  "");

        config("psep_querystr",   "?");
        config("vsep_querystr",   "");

        // not allowed here
        config("rx_username",     "");
        config("rx_password",     "");
        config("rx_fragment",     "");
        config("rx_pathparams",   "");
      }

      virtual zypp::url::ParamMap
      getQueryStringMap(zypp::url::EEncoding eflag) const
      {
        static const char * const keys[] = {
          "attrs", "scope", "filter", "exts", NULL
        };
        zypp::url::ParamMap pmap;
        zypp::url::ParamVec pvec( getQueryStringVec());
        if( pvec.size() <= 4)
        {
          for(size_t i=0; i<pvec.size(); i++)
          {
            if(eflag == zypp::url::E_ENCODED)
              pmap[keys[i]] = pvec[i];
            else
              pmap[keys[i]] = zypp::url::decode( pvec[i]);
          }
        }
        else
        {
          throw std::invalid_argument(
            "Invalid LDAP URL query string"
          );
        }
        return pmap;
      }

      virtual void
      setQueryStringMap(const zypp::url::ParamMap &pmap)
      {
        static const char * const keys[] = {
          "attrs", "scope", "filter", "exts", NULL
        };

        // remove psep ("?") from safe chars
        std::string join_safe;
        std::string safe(config("safe_querystr"));
        std::string psep(config("psep_querystr"));
        for(std::string::size_type i=0; i<safe.size(); i++)
        {
          if( psep.find(safe[i]) == std::string::npos)
            join_safe.append(1, safe[i]);
        }

        zypp::url::ParamVec pvec(4);
        zypp::url::ParamMap::const_iterator p;
        for(p=pmap.begin(); p!=pmap.end(); ++p)
        {
          bool found=false;
          for(size_t i=0; i<4; i++)
          {
            if(p->first == keys[i])
            {
              found=true;
              pvec[i] = zypp::url::encode(p->second, join_safe);
            }
          }
          if( !found)
          {
            throw std::invalid_argument(
              "Invalid LDAP URL query parameter '" + p->first + "'"
            );
          }
        }
        setQueryStringVec(pvec);
      }
    };


    // ---------------------------------------------------------------
    // FIXME: hmm..
    class UrlByScheme
    {
    private:
      typedef std::map<std::string,UrlRef> UrlBySchemeMap;
      UrlBySchemeMap urlByScheme;

    public:
      UrlByScheme()
      {
        UrlRef ref;

        ref.reset( new LDAPUrl());
        addUrlByScheme("ldap", ref);
        addUrlByScheme("ldaps", ref);

        ref.reset( new UrlBase());
        // don't show empty authority
        ref->setViewOptions( zypp::url::ViewOption::DEFAULTS -
                             zypp::url::ViewOption::EMPTY_AUTHORITY);
        ref->config("rx_username",      "");  // disallow username
        ref->config("rx_password",      "");  // disallow password
        // FIXME: hmm... also host+port?
        addUrlByScheme("nfs",    ref);

        ref->config("with_authority"    "n");   // disallow host & port
        addUrlByScheme("mailto", ref);
      }

      bool
      addUrlByScheme(const std::string &scheme,
                     UrlRef            urlImpl)
      {
        if( urlImpl && urlImpl->isValidScheme(scheme))
        {
          UrlRef ref(urlImpl);
          ref->clear();
          urlByScheme[str::toLower(scheme)] = ref;
          return true;
        }
        return false;
      }

      UrlRef
      getUrlByScheme(const std::string &scheme) const
      {
        UrlBySchemeMap::const_iterator i(urlByScheme.find(str::toLower(scheme)));
        if( i != urlByScheme.end())
        {
          return i->second;
        }
        return UrlRef();
      }

      bool
      isRegisteredScheme(const std::string &scheme) const
      {
        return urlByScheme.find(str::toLower(scheme)) != urlByScheme.end();
      }

      UrlSchemes
      getRegisteredSchemes() const
      {
        UrlBySchemeMap::const_iterator i(urlByScheme.begin());
        UrlSchemes                     schemes;

        schemes.reserve(urlByScheme.size());
        for( ; i != urlByScheme.begin(); ++i)
        {
          schemes.push_back(i->first);
        }
        return schemes;
      }
    };


    // ---------------------------------------------------------------
    UrlByScheme g_urlSchemeRepository;


    //////////////////////////////////////////////////////////////////
  } // anonymous namespace
  ////////////////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  Url::~Url()
  {
  }


  // -----------------------------------------------------------------
  Url::Url()
    : m_impl( new UrlBase())
  {
  }


  // -----------------------------------------------------------------
  Url::Url(const Url &url)
    : m_impl( url.m_impl)
  {
    if( !m_impl)
    {
      throw std::invalid_argument(
        "Unable to clone Url object"
      );
    }
  }


  // -----------------------------------------------------------------
  Url::Url(const zypp::url::UrlRef &url)
    : m_impl( url)
  {
    if( !m_impl)
    {
      throw std::invalid_argument(
        "Invalid empty Url reference"
      );
    }
  }


  // -----------------------------------------------------------------
  Url::Url(const std::string &encodedUrl)
    : m_impl( parseUrl(encodedUrl))
  {
    if( !m_impl)
    {
      throw std::invalid_argument(
        "Unable to parse Url components"
      );
    }
  }


  // -----------------------------------------------------------------
  Url&
  Url::operator = (const std::string &encodedUrl)
  {
    UrlRef  url( parseUrl(encodedUrl));
    if( !url)
    {
      throw std::invalid_argument(
        "Unable to parse Url components"
      );
    }
    m_impl = url;
    return *this;
  }


  // -----------------------------------------------------------------
  Url&
  Url::operator = (const Url &url)
  {
    m_impl = url.m_impl;
    return *this;
  }


  // -----------------------------------------------------------------
  // static
  bool
  Url::registerScheme(const std::string &scheme,
                      UrlRef            urlImpl)
  {
    return g_urlSchemeRepository.addUrlByScheme(scheme, urlImpl);
  }


  // -----------------------------------------------------------------
  // static
  UrlRef
  Url::parseUrl(const std::string &encodedUrl)
  {
    UrlRef      url;
    str::smatch out;
    bool        ret = false;

    try
    {
      str::regex  rex(RX_SPLIT_URL);
      ret = str::regex_match(encodedUrl, out, rex);
    }
    catch( ... )
    {}

    if(ret && out.size() == 10)
    {
      url = g_urlSchemeRepository.getUrlByScheme(out[2].str());
      if( !url)
      {
        url.reset( new UrlBase());
      }
      url->init(out[2].str(),
                out[4].str(),
                out[5].str(),
                out[7].str(),
                out[9].str());
    }
    return url;
  }


  // -----------------------------------------------------------------
  // static
  zypp::url::UrlSchemes
  Url::getRegisteredSchemes()
  {
    return g_urlSchemeRepository.getRegisteredSchemes();
  }


  // -----------------------------------------------------------------
  // static
  bool
  Url::isRegisteredScheme(const std::string &scheme)
  {
    return g_urlSchemeRepository.isRegisteredScheme(scheme);
  }


  // -----------------------------------------------------------------
  zypp::url::UrlSchemes
  Url::getKnownSchemes() const
  {
    return m_impl->getKnownSchemes();
  }


  // -----------------------------------------------------------------
  bool
  Url::isValidScheme(const std::string &scheme) const
  {
    return m_impl->isValidScheme(scheme);
  }


  // -----------------------------------------------------------------
  bool
  Url::isValid() const
  {
    return m_impl->isValid();
  }


  // -----------------------------------------------------------------
  std::string
  Url::asString() const
  {
    return m_impl->asString();
  }


  // -----------------------------------------------------------------
  std::string
  Url::asString(const ViewOptions &opts) const
  {
    return m_impl->asString(opts);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getScheme() const
  {
    return m_impl->getScheme();
  }


  // -----------------------------------------------------------------
  std::string
  Url::getAuthority() const
  {
    return m_impl->getAuthority();
  }

  // -----------------------------------------------------------------
  std::string
  Url::getPathData() const
  {
    return m_impl->getPathData();
  }


  // -----------------------------------------------------------------
  std::string
  Url::getQueryString() const
  {
    return m_impl->getQueryString();
  }


  // -----------------------------------------------------------------
  std::string
  Url::getFragment(zypp::url::EEncoding eflag) const
  {
    return m_impl->getFragment(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getUsername(EEncoding eflag) const
  {
    return m_impl->getUsername(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getPassword(EEncoding eflag) const
  {
    return m_impl->getPassword(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getHost(EEncoding eflag) const
  {
    return m_impl->getHost(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getPort() const
  {
    return m_impl->getPort();
  }


  // -----------------------------------------------------------------
  std::string
  Url::getPathName(EEncoding eflag) const
  {
    return m_impl->getPathName(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getPathParams() const
  {
    return m_impl->getPathParams();
  }


  // -----------------------------------------------------------------
  zypp::url::ParamVec
  Url::getPathParamsVec() const
  {
    return m_impl->getPathParamsVec();
  }


  // -----------------------------------------------------------------
  zypp::url::ParamMap
  Url::getPathParamsMap(EEncoding eflag) const
  {
    return m_impl->getPathParamsMap(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getPathParam(const std::string &param, EEncoding eflag) const
  {
    return m_impl->getPathParam(param, eflag);
  }


  // -----------------------------------------------------------------
  zypp::url::ParamVec
  Url::getQueryStringVec() const
  {
    return m_impl->getQueryStringVec();
  }


  // -----------------------------------------------------------------
  zypp::url::ParamMap
  Url::getQueryStringMap(EEncoding eflag) const
  {
    return m_impl->getQueryStringMap(eflag);
  }


  // -----------------------------------------------------------------
  std::string
  Url::getQueryParam(const std::string &param, EEncoding eflag) const
  {
    return m_impl->getQueryParam(param, eflag);
  }


  // -----------------------------------------------------------------
  void
  Url::setScheme(const std::string &scheme)
  {
    if(scheme == m_impl->getScheme())
    {
      return;
    }
    if( m_impl->isKnownScheme(scheme))
    {
      m_impl->setScheme(scheme);
      return;
    }

    UrlRef url = g_urlSchemeRepository.getUrlByScheme(scheme);
    if( !url)
    {
      url.reset( new UrlBase());
    }
    url->init(
      scheme,
      m_impl->getAuthority(),
      m_impl->getPathData(),
      m_impl->getQueryString(),
      m_impl->getFragment(zypp::url::E_ENCODED)
    );
    m_impl = url;
  }


  // -----------------------------------------------------------------
  void
  Url::setAuthority(const std::string &authority)
  {
    m_impl->setAuthority(authority);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathData(const std::string &pathdata)
  {
    m_impl->setPathData(pathdata);
  }


  // -----------------------------------------------------------------
  void
  Url::setQueryString(const std::string &querystr)
  {
    m_impl->setQueryString(querystr);
  }


  // -----------------------------------------------------------------
  void
  Url::setFragment(const std::string &fragment, EEncoding eflag)
  {
    m_impl->setFragment(fragment, eflag);
  }


  // -----------------------------------------------------------------
  void
  Url::setUsername(const std::string &user,
                   EEncoding         eflag)
  {
    m_impl->setUsername(user, eflag);
  }


  // -----------------------------------------------------------------
  void
  Url::setPassword(const std::string &pass,
                   EEncoding         eflag)
  {
    m_impl->setPassword(pass, eflag);
  }


  // -----------------------------------------------------------------
  void
  Url::setHost(const std::string &host)
  {
    m_impl->setHost(host);
  }


  // -----------------------------------------------------------------
  void
  Url::setPort(const std::string &port)
  {
    m_impl->setPort(port);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathName(const std::string &path,
                   EEncoding         eflag)
  {
    m_impl->setPathName(path, eflag);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathParams(const std::string &params)
  {
    m_impl->setPathParams(params);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathParamsVec(const zypp::url::ParamVec &pvec)
  {
    m_impl->setPathParamsVec(pvec);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathParamsMap(const zypp::url::ParamMap &pmap)
  {
    m_impl->setPathParamsMap(pmap);
  }


  // -----------------------------------------------------------------
  void
  Url::setPathParam(const std::string &param, const std::string &value)
  {
    m_impl->setPathParam(param, value);
  }


  // -----------------------------------------------------------------
  void
  Url::setQueryStringVec(const zypp::url::ParamVec &pvec)
  {
    m_impl->setQueryStringVec(pvec);
  }


  // -----------------------------------------------------------------
  void
  Url::setQueryStringMap(const zypp::url::ParamMap &pmap)
  {
    m_impl->setQueryStringMap(pmap);
  }

  // -----------------------------------------------------------------
  void
  Url::setQueryParam(const std::string &param, const std::string &value)
  {
    m_impl->setQueryParam(param, value);
  }


  // -----------------------------------------------------------------
  std::ostream & operator<<( std::ostream & str, const Url & url )
  {
    return str << url.asString();
  }


  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
