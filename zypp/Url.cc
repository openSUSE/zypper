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
// FIXME:
#if defined(WITH_URL_PARSE_DEBUG)
#include <iostream>
#endif

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////


  using namespace zypp::url;


  // -----------------------------------------------------------------
  /**
   * url       = [scheme:] [//authority] /path [?query] [#fragment]
   */
  #define RX_SPLIT_URL                       "^(([^:/?#]+):)?" \
                                             "(//([^/?#]*))?"  \
                                             "([^?#]*)"        \
                                             "([?]([^#]*))?"   \
                                             "(#(.*))?"


  // -----------------------------------------------------------------
  // FIXME:
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

    virtual UrlBase::Schemes
    getKnownSchemes() const
    {
      UrlBase::Schemes schemes(2);
      schemes[0] = "ldap";
      schemes[1] = "ldaps";
      return schemes;
    }

    virtual void
    configure()
    {
      // => put base-dn to path params :-)
      // ahm... will this work propelly?
      //config("sep_pathparams", "/");

      config("psep_querystr",  "?");
      config("vsep_querystr",  "");

      config("safe_pathname",  "/=,");
      config("safe_querystr",  "(),=");

      // not allowed here
      config("rx_username",     "");
      config("rx_password",     "");
      config("rx_fragment",     "");
    }
  };


  // -----------------------------------------------------------------
  // FIXME:
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

      // FIXME: ok?
      ref.reset( new UrlBase());
      ref->config("empty_authority",  "n");
      ref->config("rx_username",      "");
      ref->config("rx_password",      "");
      addUrlByScheme("nfs",    ref);

      ref->config("safe_pathname",    "@");
      ref->config("rx_hostname",      "");
      ref->config("rx_port",          "");
      addUrlByScheme("mailto", ref);
    }

    bool
    addUrlByScheme(const std::string &scheme,
                   UrlRef            urlImpl)
    {
      if( !scheme.empty() && urlImpl &&
          urlImpl->isValidScheme(scheme))
      {
        UrlRef ref(urlImpl);
        ref->clear();
        urlByScheme[scheme] = ref;
        return true;
      }
      return false;
    }

    UrlRef
    getUrlByScheme(const std::string &scheme) const
    {
      UrlBySchemeMap::const_iterator i(urlByScheme.find(scheme));
      if( i != urlByScheme.end())
      {
        return i->second;
      }
      return UrlRef();
    }

    UrlBase::Schemes
    getKnownSchemes() const
    {
      UrlBySchemeMap::const_iterator i(urlByScheme.begin());
      UrlBase::Schemes               schemes;

      schemes.reserve(urlByScheme.size());
      for( ; i != urlByScheme.begin(); ++i)
      {
        schemes.push_back(i->first);
      }
      return schemes;
    }
  };


  // -----------------------------------------------------------------
  UrlByScheme g_urlSchemeRepository;


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
    str::regex  rex(RX_SPLIT_URL);
    str::smatch out;
    bool        ret = str::regex_match(encodedUrl, out, rex);

    // FIXME:
    #if defined(WITH_URL_PARSE_DEBUG)
    std::cerr << "Regex parsed URL components("
              << out.size() << "):" << std::endl;
    std::cerr << "URL " << encodedUrl << std::endl;
    for(size_t n=0; n < out.size(); n++)
    {
      std::cerr << "[" << n << "] " << out[n].str() << std::endl;
    }
    #endif

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
  UrlBase::Schemes
  Url::getAllKnownSchemes()
  {
    return g_urlSchemeRepository.getKnownSchemes();
  }


  // -----------------------------------------------------------------
  UrlBase::Schemes
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
  std::string
  Url::toString() const
  {
    return m_impl->toString();
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
  Url::setFragment(const std::string &fragment)
  {
    m_impl->setFragment(fragment);
  }


  // -----------------------------------------------------------------
  void
  Url::setUsername(const std::string &user)
  {
    m_impl->setUsername(user);
  }


  // -----------------------------------------------------------------
  void
  Url::setPassword(const std::string &pass)
  {
    m_impl->setPassword(pass);
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
  Url::setPathName(const std::string &path)
  {
    m_impl->setPathName(path);
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


  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
