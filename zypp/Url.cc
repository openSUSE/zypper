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

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"
#include <stdexcept>
#include <iostream>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////


  using namespace zypp::url;


  // -----------------------------------------------------------------
  /*
   * url       = [scheme:] [//authority] /path [?query] [#fragment]
   */
  #define RX_SPLIT_URL                       "^([^:/?#]+:|)" \
                                             "(//[^/?#]*|)"  \
                                             "([^?#]*)"        \
                                             "([?][^#]*|)"   \
                                             "(#.*|)"


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

        // host is required (isValid=>false)
        // but not mandatory (see RFC 2255),
        // that is, accept empty host.
        config("require_host",    "y");

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
          ZYPP_THROW(url::UrlNotSupportedException(
            _("Invalid LDAP URL query string")
          ));
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
            ZYPP_THROW(url::UrlNotSupportedException(
              str::form(_("Invalid LDAP URL query parameter '%s'"),
                          p->first.c_str())
            ));
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

        // =====================================
        ref.reset( new LDAPUrl());
        addUrlByScheme("ldap", ref);
        addUrlByScheme("ldaps", ref);


        // =====================================
        ref.reset( new UrlBase());
        ref->config("with_authority",   "n");   // disallow host,...
        ref->config("require_pathname", "m");   // path is mandatory
        addUrlByScheme("hd",     ref);
        addUrlByScheme("cd",     ref);
        addUrlByScheme("dvd",    ref);
        addUrlByScheme("dir",    ref);
        addUrlByScheme("iso",    ref);

        // don't show empty authority
        ref->setViewOptions( zypp::url::ViewOption::DEFAULTS -
                             zypp::url::ViewOption::EMPTY_AUTHORITY);
        addUrlByScheme("mailto", ref);
        addUrlByScheme("urn",    ref);
        addUrlByScheme("plugin", ref);	// zypp plugable media handler:

        // RFC1738, 3.10: may contain a host
        ref->config("with_authority",   "y");   // allow host,
        ref->config("with_port",        "n");   // but no port,
        ref->config("rx_username",      "");    // username or
        ref->config("rx_password",      "");    // password ...
        addUrlByScheme("file",   ref);

        // =====================================
        ref.reset( new UrlBase());
        ref->config("require_host",     "m");   // host is mandatory
        addUrlByScheme("nfs",    ref);
        addUrlByScheme("nfs4",   ref);
        addUrlByScheme("smb",    ref);
        addUrlByScheme("cifs",   ref);
        addUrlByScheme("http",   ref);
        addUrlByScheme("https",  ref);
        ref->config("path_encode_slash2", "y"); // always encode 2. slash
        addUrlByScheme("ftp",    ref);
        addUrlByScheme("sftp",   ref);
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
        for( ; i != urlByScheme.end(); ++i)
        {
          schemes.push_back(i->first);
        }
        return schemes;
      }
    };


    // ---------------------------------------------------------------
    UrlByScheme & g_urlSchemeRepository()
    {
      static UrlByScheme _v;
      return _v;
    }

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
      ZYPP_THROW(url::UrlException(
        _("Unable to clone Url object")
      ));
    }
  }


  // -----------------------------------------------------------------
  Url::Url(const zypp::url::UrlRef &url)
    : m_impl( url)
  {
    if( !m_impl)
    {
      ZYPP_THROW(url::UrlException(
        _("Invalid empty Url object reference")
      ));
    }
  }


  // -----------------------------------------------------------------
  Url::Url(const std::string &encodedUrl)
    : m_impl( parseUrl(encodedUrl))
  {
    if( !m_impl)
    {
      ZYPP_THROW(url::UrlParsingException(
        _("Unable to parse Url components")
      ));
    }
  }


  // -----------------------------------------------------------------
  Url&
  Url::operator = (const std::string &encodedUrl)
  {
    UrlRef url( parseUrl(encodedUrl));
    if( !url)
    {
      ZYPP_THROW(url::UrlParsingException(
        _("Unable to parse Url components")
      ));
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
    return g_urlSchemeRepository().addUrlByScheme(scheme, urlImpl);
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

    if(ret && out.size() == 6)
    {
      std::string scheme = out[1];
      if (scheme.size() > 1)
        scheme = scheme.substr(0, scheme.size()-1);
      std::string authority = out[2];
      if (authority.size() >= 2)
        authority = authority.substr(2);
      std::string query = out[4];
      if (query.size() > 1)
        query = query.substr(1);
      std::string fragment = out[5];
      if (fragment.size() > 1)
        fragment = fragment.substr(1);

      url = g_urlSchemeRepository().getUrlByScheme(scheme);
      if( !url)
      {
        url.reset( new UrlBase());
      }
      url->init(scheme, authority, out[3],
                query, fragment);
    }
    return url;
  }


  // -----------------------------------------------------------------
  // static
  zypp::url::UrlSchemes
  Url::getRegisteredSchemes()
  {
    return g_urlSchemeRepository().getRegisteredSchemes();
  }


  // -----------------------------------------------------------------
  // static
  bool
  Url::isRegisteredScheme(const std::string &scheme)
  {
    return g_urlSchemeRepository().isRegisteredScheme(scheme);
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


  ///////////////////////////////////////////////////////////////////
  namespace
  {
    inline bool isInList( const char ** begin_r, const char ** end_r, const std::string & scheme_r )
    {
      for ( ; begin_r != end_r; ++begin_r )
        if ( scheme_r == *begin_r )
          return true;
      return false;
    }
  }
  bool Url::schemeIsLocal( const std::string & scheme_r )
  {
    static const char * val[] = { "cd", "dvd", "dir", "hd", "iso", "file" };
    return isInList( arrayBegin(val), arrayEnd(val), scheme_r );
  }

  bool Url::schemeIsRemote( const std::string & scheme_r )
  {
    static const char * val[] = { "http", "https", "nfs", "nfs4", "smb", "cifs", "ftp", "sftp" };
    return isInList( arrayBegin(val), arrayEnd(val), scheme_r );
  }

  bool Url::schemeIsVolatile( const std::string & scheme_r )
  {
    static const char * val[] = { "cd", "dvd" };
    return isInList( arrayBegin(val), arrayEnd(val), scheme_r );
  }

  bool Url::schemeIsDownloading( const std::string & scheme_r )
  {
    static const char * val[] = { "http", "https", "ftp", "sftp" };
    return isInList( arrayBegin(val), arrayEnd(val), scheme_r );
  }
  ///////////////////////////////////////////////////////////////////

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
  Url::asCompleteString() const
  {
    // make sure, all url components are included;
    // regardless of the current configuration...
    ViewOptions opts(getViewOptions() +
                     ViewOption::WITH_SCHEME +
                     ViewOption::WITH_USERNAME +
                     ViewOption::WITH_PASSWORD +
                     ViewOption::WITH_HOST +
                     ViewOption::WITH_PORT +
                     ViewOption::WITH_PATH_NAME +
                     ViewOption::WITH_PATH_PARAMS +
                     ViewOption::WITH_QUERY_STR +
                     ViewOption::WITH_FRAGMENT);
    return m_impl->asString(opts);
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

    UrlRef url = g_urlSchemeRepository().getUrlByScheme(scheme);
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

  void
  Url::setPathName(const Pathname &path,
                   EEncoding         eflag)
  {
    m_impl->setPathName(path.asString(), eflag);
  }

  void
  Url::setPathName(const char *path,
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
  void
  Url::delQueryParam(const std::string &param)
  {
    m_impl->delQueryParam(param);
  }

  // -----------------------------------------------------------------
  ViewOptions
  Url::getViewOptions() const
  {
    return m_impl->getViewOptions();
  }

  // -----------------------------------------------------------------
  void
  Url::setViewOptions(const ViewOptions &vopts)
  {
    m_impl->setViewOptions(vopts);
  }

  // -----------------------------------------------------------------
  std::ostream & operator<<( std::ostream & str, const Url & url )
  {
    return str << url.asString();
  }

  bool operator<( const Url &lhs, const Url &rhs )
  {
    return (lhs.asCompleteString() < rhs.asCompleteString());
  }

  bool operator==( const Url &lhs, const Url &rhs )
  {
    return (lhs.asCompleteString() == rhs.asCompleteString());
  }

  bool operator!=( const Url &lhs, const Url &rhs )
  {
    return (lhs.asCompleteString() != rhs.asCompleteString());
  }

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
