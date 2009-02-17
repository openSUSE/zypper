#include <iostream>
#include <vector>
#include <sstream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/WatchFile.h"
#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/ExternalProgram.h"
#include "zypp/media/TransferProgram.h"

using namespace std;

#define ARIA2C_BINARY "/usr/bin/aria2c"
#define CURL_BINARY "/usr/bin/curl"

namespace zypp
{
namespace media
{
    
class TransferProgram::Impl
{
public:
    Impl()
        : _useproxy(false)
        , _timeout(0)
    {}

    virtual ~Impl()
    {}    

    virtual void execute()
    {
        ERR << "not implemented." << endl;
        ZYPP_THROW(Exception("not implemented"));
    }
    
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }

private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }

public:
    vector<string> _headers;
    string _useragent;
    string _username;
    string _password;
    bool _useproxy;
    string _proxy_username;
    string _proxy_password;
    int _timeout;
    Url _url;

    TransferProgram::OutputLines _output;
    ErrorFlags _errorFlags;
    int _exitCode;
};
    
class Aria2cProgram : public TransferProgram::Impl
{
public:
    virtual void execute()
    {
        ExternalProgram::Arguments args;
    
        args.push_back(ARIA2C_BINARY);
        args.push_back(str::form("--user-agent=%s", _useragent.c_str()));
        args.push_back("--summary-interval=1");
        args.push_back("--follow-metalink=mem");
        args.push_back("--check-integrity=true");

        // add the anonymous id.
        for ( vector<string>::const_iterator it = _headers.begin();
              it != _headers.end();
              ++it )
            args.push_back(str::form("--header=%s", it->c_str() ));
        
        args.push_back( str::form("--connect-timeout=%d", _timeout));

        ExternalProgram prog(args,ExternalProgram::Discard_Stderr, false, -1, true);
        string line;
        int count;
        for(line = prog.receiveLine(), count=0;
            !line.empty();
            line = prog.receiveLine(), count++ )
        {
            // save the line
            _output.push_back(line);
            
            // trim trailing NL.
            if ( line.empty() )
                continue;
            // make sure no \n is at the end
            line = str::rtrim(line);

            // process lines
        }
        _exitCode = prog.close();

        // clear previous argument
        args.clear();
    }
};
    
class CurlProgram : public TransferProgram::Impl
{
public:
    virtual void execute()
    {
    }
};


TransferProgram::TransferProgram()
    : _impl()
{

}

TransferProgram::OutputLines::const_iterator TransferProgram::outputLinesBegin() const
{
    return _impl->_output.begin();
}

TransferProgram::OutputLines::const_iterator TransferProgram::outputLinesEnd() const
{
    return _impl->_output.end();
}

string TransferProgram::output() const
{
    stringstream str;
    OutputLines::const_iterator it;
    for ( it = outputLinesBegin();
          it != outputLinesEnd();
          ++it )
    {
        str << *it;
    }
    return str.str();
}    

void TransferProgram::execute()
{
    _impl->execute();
}

void TransferProgram::addHeader( const std::string &header )
{
    _impl->_headers.push_back(header);
}

void TransferProgram::setUserAgentString( const std::string &agent )
{
    _impl->_useragent = agent;
}

void TransferProgram::setUsername( const std::string &username )
{
    _impl->_username = username;
}

void TransferProgram::setPassword( const std::string &password )
{
    _impl->_password = password;
}

void TransferProgram::setProxyEnabled( bool enabled )
{
    _impl->_useproxy = enabled;
}

void TransferProgram::setProxyUsername( const std::string &proxyuser )
{
    _impl->_proxy_username = proxyuser;
}

void TransferProgram::setProxyPassword( const std::string &proxypass )
{
    _impl->_proxy_password = proxypass;
}

void TransferProgram::setTimeout( int t )
{
    _impl->_timeout = t;
}

void TransferProgram::setUrl( const zypp::Url &url)
{
    _impl->_url = url;
}


} // ns media
} // ns zypp

