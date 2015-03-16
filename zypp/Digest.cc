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

#include "zypp/Digest.h"

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
    	P();
    	~P();

    	EVP_MD_CTX mdctx;

    	const EVP_MD *md;
    	unsigned char md_value[EVP_MAX_MD_SIZE];
    	unsigned md_len;

    	bool initialized : 1;
    	bool finalized : 1;
    	static bool openssl_digests_added;

    	std::string name;

    	inline bool maybeInit();
    	inline void cleanup();
    };


    using namespace std;

    bool Digest::P::openssl_digests_added = false;

    Digest::P::P() :
      md(NULL),
      initialized(false),
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

      if(!initialized)
      {
    	md = EVP_get_digestbyname(name.c_str());
    	if(!md)
    	    return false;

    	EVP_MD_CTX_init(&mdctx);

    	if(!EVP_DigestInit_ex(&mdctx, md, NULL))
    	    return false;

    	md_len = 0;
    	::memset(md_value, 0, sizeof(md_value));
    	initialized = true;
      }
      return true;
    }

    void Digest::P::cleanup()
    {
      if(initialized)
      {
    	EVP_MD_CTX_cleanup(&mdctx);
    	initialized = false;
    	finalized = false;
      }
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

      if(_dp->initialized)
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
      if (!_dp->initialized)
	return false;
      if(!_dp->finalized)
	{
	  (void)EVP_DigestFinal_ex(&_dp->mdctx, _dp->md_value, &_dp->md_len);
          _dp->finalized = true;
	}
      if(!EVP_DigestInit_ex(&_dp->mdctx, _dp->md, NULL))
	return false;
      _dp->finalized = false;
      return true;
    }

    std::string Digest::digest()
    {
      if(!_dp->maybeInit())
    	return std::string();

      if(!_dp->finalized)
      {
    	if(!EVP_DigestFinal_ex(&_dp->mdctx, _dp->md_value, &_dp->md_len))
    	    return std::string();

    	_dp->finalized = true;
      }

      char mdtxt[_dp->md_len*2 + 1];
      mdtxt[_dp->md_len*2] = '\0';

      for(unsigned i = 0; i < _dp->md_len; ++i)
      {
    	::snprintf(mdtxt + i*2, 3, "%02hhx", _dp->md_value[i]);
      }

      return std::string(mdtxt);
    }

    std::vector<unsigned char> Digest::digestVector()
    {
      std::vector<unsigned char> r;
      if(!_dp->maybeInit())
        return r;

      if(!_dp->finalized)
      {
        if(!EVP_DigestFinal_ex(&_dp->mdctx, _dp->md_value, &_dp->md_len))
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
      if(!EVP_DigestUpdate(&_dp->mdctx, reinterpret_cast<const unsigned char*>(bytes), len))
    	return false;

      return true;
    }

    std::string Digest::digest(const std::string& name, std::istream& is, size_t bufsize)
    {
      if(name.empty() || !is)
    	return string();

      char buf[bufsize];

      Digest digest;
      if(!digest.create(name))
    	return string();


      while(is.good())
      {
        int readed;
        is.read(buf, bufsize);
    	readed = is.gcount();
        if(readed && !digest.update(buf, readed))
    	    return string();
      }

      return digest.digest();
    }

    std::string Digest::digest( const std::string & name, const std::string & input, size_t bufsize )
    {
      istringstream is( input );
      return digest( name, is, bufsize );
    }

#ifdef DIGEST_TESTSUITE
    int main(int argc, char *argv[])
    {
      bool openssl = false;
      unsigned argpos = 1;

      if(argc > 1 && string(argv[argpos]) == "--openssl")
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

      ifstream file(fn);

      string digest = Digest::digest(digestname, file);

      if(openssl)
    	cout << digestname << "(" << fn << ")= " << digest << endl;
      else
    	cout << digest << "  " << fn << endl;

      return 0;
    }
#endif

} // namespace zypp
