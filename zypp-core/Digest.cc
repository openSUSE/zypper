/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Digest.cc
 *
 * \todo replace by Blocxx
 *
*/

#include <cstdio> // snprintf
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <string>
#include <string.h>

#include <iostream>
#include <sstream>

#ifdef DIGEST_TESTSUITE
#include <fstream>
#endif

#include <zypp/Digest.h>
#include <zypp/base/PtrTypes.h>

using std::endl;

namespace zypp {

    bool DigestReport::askUserToAcceptNoDigest( const zypp::Pathname &file )
    { return false; }

    bool DigestReport::askUserToAccepUnknownDigest( const Pathname &file, const std::string &name )
    { return false; }

    bool DigestReport::askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found )
    { return false; }


    const std::string & Digest::md5()
    { static std::string _type( "md5" ); return _type; }

    const std::string & Digest::sha1()
    { static std::string _type( "sha1" ); return _type; }

    const std::string & Digest::sha224()
    { static std::string _type( "sha224" ); return _type; }

    const std::string & Digest::sha256()
    { static std::string _type( "sha256" ); return _type; }

    const std::string & Digest::sha384()
    { static std::string _type( "sha384" ); return _type; }

    const std::string & Digest::sha512()
    { static std::string _type( "sha512" ); return _type; }

    // private data
    class Digest::P
    {
      P(const P& p);
      const P& operator=(const P& p);

      public:
        typedef zypp::shared_ptr<EVP_MD_CTX> EvpDataPtr;
        P();
        ~P();

        EvpDataPtr mdctx;

        const EVP_MD *md;
        unsigned char md_value[EVP_MAX_MD_SIZE];
        unsigned md_len;

        bool finalized : 1;
        static bool openssl_digests_added;

        std::string name;

        inline bool maybeInit();
        inline void cleanup();
    };



    bool Digest::P::openssl_digests_added = false;

    Digest::P::P() :
      md(NULL),
      finalized(false)
    {
    }

    Digest::P::~P()
    {
      cleanup();
    }

    bool Digest::P::maybeInit()
    {
      if(!openssl_digests_added)
      {
        OPENSSL_config(NULL);
        ENGINE_load_builtin_engines();
        ENGINE_register_all_complete();
        OpenSSL_add_all_digests();
        openssl_digests_added = true;
      }

      if(!mdctx)
      {
        md = EVP_get_digestbyname(name.c_str());
        if(!md)
          return false;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EvpDataPtr tmp_mdctx(EVP_MD_CTX_create(), EVP_MD_CTX_destroy);
#else
        EvpDataPtr tmp_mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
#endif
        if (!tmp_mdctx)
          return false;

        if (!EVP_DigestInit_ex(tmp_mdctx.get(), md, NULL)) {
          return false;
        }

        md_len = 0;
        ::memset(md_value, 0, sizeof(md_value));

        mdctx.swap(tmp_mdctx);
      }
      return true;
    }

    void Digest::P::cleanup()
    {
      mdctx.reset();
      finalized = false;
    }

    Digest::Digest() : _dp(new P())
    {
    }

    Digest::~Digest()
    {
      delete _dp;
    }

    bool Digest::create(const std::string& name)
    {
      if(name.empty()) return false;

      if(_dp->mdctx)
    	_dp->cleanup();

      _dp->name = name;

      return _dp->maybeInit();
    }

    const std::string& Digest::name()
    {
      return _dp->name;
    }

    bool Digest::reset()
    {
      if (!_dp->mdctx)
        return false;
      if(!_dp->finalized)
      {
        (void)EVP_DigestFinal_ex(_dp->mdctx.get(), _dp->md_value, &_dp->md_len);
        _dp->finalized = true;
      }
      if(!EVP_DigestInit_ex(_dp->mdctx.get(), _dp->md, NULL))
        return false;
      _dp->finalized = false;
      return true;
    }

    std::string Digest::digest()
    {
      return digestVectorToString( digestVector() );
    }

    std::string Digest::digestVectorToString(const UByteArray &vec)
    {
      if ( vec.empty() )
        return std::string();

      std::vector<char> resData ( vec.size()*2 + 1, '\0' );
      char *mdtxt = &resData[0];
      for(unsigned i = 0; i < vec.size(); ++i)
      {
        ::snprintf( mdtxt+(i*2), 3, "%02hhx", vec[i]);
      }
      return std::string( resData.data() );
    }

#ifdef __cpp_lib_string_view
    namespace {
      template <typename BArr>
      BArr hexStrToBArr ( std::string_view &&str ) {
	BArr bytes;
	for ( std::string::size_type i = 0; i < str.length(); i+=2 )
	{
	  #define c2h(c) (((c)>='0' && (c)<='9') ? ((c)-'0')              \
	  : ((c)>='a' && (c)<='f') ? ((c)-('a'-10))       \
	  : ((c)>='A' && (c)<='F') ? ((c)-('A'-10))       \
	  : -1)
	  int v = c2h(str[i]);
	  if (v < 0)
	    return {};
	  bytes.push_back(v);
	  v = c2h(str[i+1]);
	  if (v < 0)
	    return {};
	  bytes.back() = (bytes.back() << 4) | v;
	  #undef c2h
	}
	return bytes;
      }
    } // namespace

    ByteArray Digest::hexStringToByteArray(std::string_view str)
    {
      return hexStrToBArr<ByteArray>( std::move(str) );
    }

    UByteArray Digest::hexStringToUByteArray( std::string_view str )
    {
      return hexStrToBArr<UByteArray>( std::move(str) );
    }
#endif

    UByteArray Digest::digestVector()
    {
      UByteArray r;
      if(!_dp->maybeInit())
        return r;

      if(!_dp->finalized)
      {
        if(!EVP_DigestFinal_ex(_dp->mdctx.get(), _dp->md_value, &_dp->md_len))
            return r;
        _dp->finalized = true;
      }
      r.reserve(_dp->md_len);
      for(unsigned i = 0; i < _dp->md_len; ++i)
	r.push_back(_dp->md_value[i]);
      return r;
    }

    bool Digest::update(const char* bytes, size_t len)
    {
      if(!bytes)
      {
    	return false;
      }

      if(!_dp->maybeInit())
    	return false;

      if(_dp->finalized)
      {
    	_dp->cleanup();
    	if(!_dp->maybeInit())
    	    return false;

      }
      if(!EVP_DigestUpdate(_dp->mdctx.get(), reinterpret_cast<const unsigned char*>(bytes), len))
    	return false;

      return true;
    }

    bool Digest::update(std::istream &is, size_t bufsize)
    {
      if( !is )
        return false;

      char buf[bufsize];

      while(is.good())
      {
        size_t readed;
        is.read(buf, bufsize);
        readed = is.gcount();
        if(readed && !update(buf, readed))
          return false;
      }

      return true;
    }

    std::string Digest::digest(const std::string& name, std::istream& is, size_t bufsize)
    {
      if(name.empty() || !is)
    	return std::string();

      Digest digest;
      if(!digest.create(name))
        return std::string();

      if ( !digest.update( is, bufsize ))
        return std::string();

      return digest.digest();
    }

    std::string Digest::digest( const std::string & name, const std::string & input, size_t bufsize )
    {
      std::istringstream is( input );
      return digest( name, is, bufsize );
    }

#ifdef DIGEST_TESTSUITE
    int main(int argc, char *argv[])
    {
      bool openssl = false;
      unsigned argpos = 1;

      if(argc > 1 && std::string(argv[argpos]) == "--openssl")
      {
    	openssl = true;
    	++argpos;
      }

      if(argc - argpos < 2)
      {
    	cerr << "Usage: " << argv[0] << " <DIGESTNAME> <FILE>" << endl;
    	return 1;
      }

      const char* digestname = argv[argpos++];
      const char* fn = argv[argpos++];

      std::ifstream file(fn);

      std::string digest = Digest::digest(digestname, file);

      if(openssl)
    	cout << digestname << "(" << fn << ")= " << digest << endl;
      else
    	cout << digest << "  " << fn << endl;

      return 0;
    }
#endif

} // namespace zypp
